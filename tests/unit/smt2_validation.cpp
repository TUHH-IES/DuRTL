#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/instantiation_graph.hpp>
#include <ducode/signals_data_manager.hpp>
#include <ducode/testbench.hpp>
#include <ducode/utility/VCD_utility.hpp>
#include <ducode/utility/iverilog_wrapper.hpp>
#include <ducode/utility/smt.hpp>

#include "ducode/instantiation_graph_traits.hpp"
#include "ducode/utility/simulation.hpp"
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <fmt/core.h>
#include <gsl/narrow>
#include <nlohmann/json_fwd.hpp>
#include <spdlog/spdlog.h>
#include <vcd-parser/VCDFile.hpp>
#include <vcd-parser/VCDFileParser.hpp>
#include <vcd-parser/VCDTypes.hpp>
#include <z3++.h>

#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

// use flipflop elements to identify clock signal
// use the vcd simulation data to identify input value changes
// and use that to inject values into the smt instance
// for combinatorial designs use only a single smt2 instance in the solver (no unrolling necessary)
// loop through input values in the vcd
//  - when changed value for one input detected
//  - get all input values for that timestep and inject into smt instance
TEST_CASE("simulation_to_smt_validation_basic", "[sim-smt_validation][base]") {
  nlohmann::json params;
  params["ift"] = false;
  bool smt_debug = false;

  auto test_parameters = std::make_tuple("and", "and", "", 1);
  // mux and further basic cells need to be tested here
  SECTION("Operators") {
    test_parameters = GENERATE(
        std::make_tuple("and", "and", "", 1),
        std::make_tuple("hierarchy", "hierarchy", "", 1),
        std::make_tuple("not", "not", "", 1),
        std::make_tuple("basic_gates/binary_gates_", "binary_gates_test", "", 1),
        std::make_tuple("basic_gates/mux", "mux", "", 1),
        std::make_tuple("basic_gates/unary_gates", "unary_gates_test", "", 1),
        std::make_tuple("test_cells/shift_ab_sign", "shift_ab_sign", "", 1),
        std::make_tuple("split", "split", "", 1),
        std::make_tuple("split_2", "split", "", 1),
        std::make_tuple("split_3", "split", "", 1),
        std::make_tuple("split_4", "split", "", 1),
        std::make_tuple("split_5", "split", "", 1),
        std::make_tuple("split_6", "split", "", 1),
        std::make_tuple("test_cells/pmux", "pmux", "", 1));
  }

  SECTION("Flip flops") {
    test_parameters = GENERATE(
        std::make_tuple("flipFlops/dff", "dff", "CLK", 2),
        std::make_tuple("flipFlops/dffe", "dffe", "CLK", 2),
        std::make_tuple("flipFlops/sdffe", "sdffe", "CLK", 2),
        std::make_tuple("flipFlops/sdffe_en_polarity", "sdffe_en_polarity", "CLK", 2),
        std::make_tuple("flipFlops/dffsr", "dffsr", "CLK", 2),
        std::make_tuple("flipFlops/dffsre", "dffsre", "CLK", 2),
        std::make_tuple("flipFlops/2bit_dffsre", "2bit_dffsre", "CLK", 2),
        std::make_tuple("flipFlops/2bit_dffsr", "2bit_dffsr", "CLK", 2),
        std::make_tuple("flipFlops/2bit_dffsr", "2bit_dffsr", "CLK", 2));
  }

  /* The following must fail - the SMT backend does not handle X-values

  SECTION("dff_x") {
    test_parameters = GENERATE(
      std::make_tuple("dff_design_x", "dff_design", "clk", 3));
  }

  */

  SECTION("Latches") {
    test_parameters = GENERATE(
        std::make_tuple("flipFlops/dlatch", "dlatch", "", 3),
        std::make_tuple("flipFlops/dlatchsr", "dlatchsr", "", 3),
        std::make_tuple("flipFlops/dlatchsr_1bit", "dlatchsr_1bit", "", 3),
        std::make_tuple("flipFlops/adlatch", "adlatch", "", 3));
  }

  SECTION("Simple Designs") {
    test_parameters = GENERATE(
        std::make_tuple("ff_design", "ff_design", "clk", 3),
        std::make_tuple("counter", "counter", "clk", 3)
        // std::make_tuple("pipeline_mac", "pipeline_mac")
    );
  }

  const auto &[path, design_name, clk, unroll_factor] = test_parameters;

  params["clock"] = clk;
  std::string d_name = design_name;
  std::string tb_ending = "_tb.v";
  if (d_name.contains("dff") || d_name.contains("latch")) {
    tb_ending = "_smt_tb.v";
  }
  //setup verilog simulation file paths
  auto verilog_file = boost::filesystem::path(TESTFILES_DIR) / path / fmt::format("{}.v", design_name);
  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / path / fmt::format("{}{}", design_name, tb_ending);
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / path / fmt::format("{}.json", design_name);

  spdlog::info("Considering design " + verilog_file.string());

  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);
  const uint32_t tag_size = 512;
  params["tag_size"] = tag_size;
  // read in json netlist
  auto design = ducode::Design::parse_json(json_file);
  if (clk != "") {
    design.set_clk_signal_name(clk);
  }
  // simulate design with original verilog + testbench
  auto vcd_data = ducode::simulate_design(verilog_file, testbench_file);

  // setup exported verilog file paths
  auto exported_verilog = temp_dir / "smt_design.v";
  auto exported_testbench = temp_dir / "smt_testbench.v";

  // export ift enhanced verilog design file
  design.write_verilog(exported_verilog, params);

  auto top_module = design.get_top_module();

  const VCDScope *root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));

  testbench.write_verilog(exported_testbench, params);

  spdlog::info("Simulating Exported Design + Testbench");

  auto instance = ducode::DesignInstance::create_instance(design);

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;
  auto data = ducode::simulate_design(exported_verilog, exported_testbench);
  spdlog::info("End of Simulation");
  vcd_data_vector.emplace_back(data);

  const VCDScope *root_scope_1 = &ducode::find_root_scope(data, design);
  uint32_t stepsize = 1;
  if (clk != "") {
    stepsize = gsl::narrow<uint32_t>(ducode::get_stepsize(data, clk, root_scope_1->name));
  }

  std::vector<const VCDScope *> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto &vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  //instance.write_verilog("test.v");
  //instance.write_graphviz("graph.dot");

  // z3 starts here
  z3::context ctx;
  z3::solver solver(ctx);
  // vector of net variables
  z3::expr_vector ev_edges(ctx);
  // enabling unsat core tracking
  z3::params p(ctx);
  p.set("unsat_core", true);
  solver.set(p);
  // maps input and output names to smt signal ids in ev_edges (id == index)
  std::unordered_map<std::string, uint32_t> signal_name_to_expr_id;
  // create smt instance
  for (uint32_t time = 0; time < unroll_factor; time++) {
    instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, time);
  }
  spdlog::debug("{}", solver.to_smt2());

  std::vector<std::string> hierarchy = {top_module.get_name()};
  auto input_names = top_module.get_input_names();
  std::vector<ducode::SignalIdentifier> input_ids;
  input_ids.reserve(input_names.size());
  for (auto &iname: input_names) {
    if (iname == design.get_clk_signal_name()) {
      continue;
    }
    input_ids.push_back({iname, hierarchy});
  }
  if (smt_debug) {
    CHECK(validation_breadth_first_search(instance, ev_edges, solver, ctx, input_names, signal_name_to_expr_id,
                                          unroll_factor));
  }
  CHECK(smt_sim_validation(design, instance, ev_edges, solver, ctx, input_ids, signal_name_to_expr_id, unroll_factor, stepsize));
}

TEST_CASE("simulation_to_smt_validation_async_flipflop", "[sim-smt_validation][base][.]") {
  nlohmann::json params;
  params["ift"] = false;
  bool smt_debug = false;
  std::string clk = "CLK";
  params["clock"] = clk;

  auto test_parameters = GENERATE(
      std::make_tuple("flipFlops/adff", "adff"),
      std::make_tuple("flipFlops/adffe", "adffe"),
      std::make_tuple("flipFlops/aldff", "aldff"),
      std::make_tuple("flipFlops/aldffe", "aldffe"));

  const auto &[path, design_name] = test_parameters;
  std::string d_name = design_name;
  std::string tb_ending = "_tb.v";
  if (d_name.contains("dff") || d_name.contains("latch")) {
    tb_ending = "_smt_tb.v";
  }
  //setup verilog simulation file paths
  auto verilog_file = boost::filesystem::path(TESTFILES_DIR) / path / fmt::format("{}.v", design_name);
  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / path / fmt::format("{}{}", design_name, tb_ending);
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / path / fmt::format("{}.json", design_name);

  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);
  const uint32_t tag_size = 512;
  params["tag_size"] = tag_size;
  // read in json netlist
  auto design = ducode::Design::parse_json(json_file);
  design.set_clk_signal_name(clk);
  // simulate design with original verilog + testbench
  auto vcd_data = ducode::simulate_design(verilog_file, testbench_file);

  // setup exported verilog file paths
  auto exported_verilog = temp_dir / "smt_design.v";
  auto exported_testbench = temp_dir / "smt_testbench.v";

  // export ift enhanced verilog design file
  design.write_verilog(exported_verilog, params);

  auto top_module = design.get_top_module();

  const VCDScope *root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));

  testbench.write_verilog(exported_testbench, params);

  spdlog::info("Simulating Exported Design + Testbench");

  auto instance = ducode::DesignInstance::create_instance(design);

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;
  auto data = ducode::simulate_design(exported_verilog, exported_testbench);
  spdlog::info("End of Simulation");
  vcd_data_vector.emplace_back(data);

  const VCDScope *root_scope_1 = &ducode::find_root_scope(data, design);
  auto stepsize = gsl::narrow<uint32_t>(ducode::get_stepsize(data, clk, root_scope_1->name));

  std::vector<const VCDScope *> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto &vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  //instance.write_verilog("test.v");
  //instance.write_graphviz("graph.dot");

  // z3 starts here
  z3::context ctx;
  z3::solver solver(ctx);
  // vector of net variables
  z3::expr_vector ev_edges(ctx);
  // enabling unsat core tracking
  z3::params p(ctx);
  p.set("unsat_core", true);
  solver.set(p);
  // maps input and output names to smt signal ids in ev_edges (id == index)
  std::unordered_map<std::string, uint32_t> signal_name_to_expr_id;
  // create smt instance
  const uint32_t unroll_factor = 2;
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 0);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 1);
  spdlog::debug("{}", solver.to_smt2());

  std::vector<std::string> hierarchy = {top_module.get_name()};
  auto input_names = top_module.get_input_names();
  std::vector<ducode::SignalIdentifier> input_ids;
  input_ids.reserve(input_names.size());
  for (auto &iname: input_names) {
    if (iname == design.get_clk_signal_name()) {
      continue;
    }
    input_ids.push_back({iname, hierarchy});
  }
  if (smt_debug) {
    CHECK(validation_breadth_first_search(instance, ev_edges, solver, ctx, input_names, signal_name_to_expr_id,
                                          unroll_factor));
  }
  CHECK(smt_sim_validation(design, instance, ev_edges, solver, ctx, input_ids, signal_name_to_expr_id, unroll_factor, stepsize));
}

TEST_CASE("simulation_to_smt_validation_SPI_Master", "[sim-smt_validation][advanced][.]") {
  nlohmann::json params;
  params["ift"] = false;
  bool smt_debug = false;

  auto test_parameters = GENERATE(
      std::make_tuple("SPI_Master", "SPI_Master", "clk"));

  const auto &[path, design_name, clock] = test_parameters;

  //setup verilog simulation file paths
  auto verilog_file = boost::filesystem::path(TESTFILES_DIR) / path / fmt::format("{}.v", design_name);
  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / path / fmt::format("{}_tb.v", design_name);
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / path / fmt::format("{}.json", design_name);

  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  params["clock"] = clock;
  const uint32_t tag_size = 512;
  params["tag_size"] = tag_size;
  // read in json netlist
  auto design = ducode::Design::parse_json(json_file);
  design.set_clk_signal_name("clk");
  design.set_top_module_name("SPI_Master");

  // simulate design with original verilog + testbench
  auto vcd_data = ducode::simulate_design(verilog_file, testbench_file);

  // setup exported verilog file paths
  auto exported_verilog = temp_dir / "smt_design.v";
  auto exported_testbench = temp_dir / "smt_testbench.v";

  // export ift enhanced verilog design file
  design.write_verilog(exported_verilog, params);

  auto top_module = design.get_top_module();

  auto stepsize = gsl::narrow<uint32_t>(ducode::get_stepsize(vcd_data, clock));
  params["stepsize"] = stepsize;

  const VCDScope *root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));

  testbench.write_verilog(exported_testbench, params);

  spdlog::info("Simulating Exported Design + Testbench");

  auto instance = ducode::DesignInstance::create_instance(design);

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;
  auto data = ducode::simulate_design(exported_verilog, exported_testbench);
  spdlog::info("End of Simulation");
  vcd_data_vector.emplace_back(data);

  std::vector<const VCDScope *> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto &vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  //instance.write_verilog("test.v");
  //instance.write_graphviz("graph.dot");

  // z3 starts here
  z3::context ctx;
  z3::solver solver(ctx);
  // vector of net variables
  z3::expr_vector ev_edges(ctx);
  // enabling unsat core tracking
  z3::params p(ctx);
  p.set("unsat_core", true);
  solver.set(p);
  // maps input and output names to smt signal ids in ev_edges (id == index)
  std::unordered_map<std::string, uint32_t> signal_name_to_expr_id;
  // create smt instance

  const uint32_t unroll_factor = 3;
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 0);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 1);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 2);
  spdlog::debug("{}", solver.to_smt2());
  ducode::DesignInstance::write_smt2(solver, "smt_spi_master.smt2");
  std::vector<std::string> hierarchy = {top_module.get_name()};
  auto input_names = top_module.get_input_names();
  std::vector<ducode::SignalIdentifier> input_ids;
  input_ids.reserve(input_names.size());
  for (auto &iname: input_names) {
    if (iname == design.get_clk_signal_name()) {
      continue;
    }
    input_ids.push_back({iname, hierarchy});
  }
  if (smt_debug) {
    CHECK(validation_breadth_first_search(instance, ev_edges, solver, ctx, input_names, signal_name_to_expr_id,
                                          unroll_factor));
  }
  CHECK(smt_sim_validation(design, instance, ev_edges, solver, ctx, input_ids, signal_name_to_expr_id, unroll_factor, stepsize));
}

TEST_CASE("simulation_to_smt_validation_y86", "[sim-smt_validation][advanced][.]") {
  nlohmann::json params;
  params["ift"] = false;
  bool smt_debug = false;

  auto test_parameters = GENERATE(
      std::make_tuple("y86", "y86", "clk"));

  const auto &[path, design_name, clock] = test_parameters;

  //setup verilog simulation file paths
  auto verilog_file = boost::filesystem::path(TESTFILES_DIR) / path / fmt::format("{}.v", design_name);
  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / path / fmt::format("{}_tb.v", design_name);
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / path / fmt::format("{}.json", design_name);

  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  params["clock"] = clock;
  const uint32_t tag_size = 512;
  params["tag_size"] = tag_size;
  // read in json netlist
  auto design = ducode::Design::parse_json(json_file);
  design.set_clk_signal_name(clock);
  design.set_top_module_name("cisc_cpu_p");

  // simulate design with original verilog + testbench
  auto vcd_data = ducode::simulate_design(verilog_file, testbench_file);

  // setup exported verilog file paths
  auto exported_verilog = temp_dir / "smt_design.v";
  auto exported_testbench = temp_dir / "smt_testbench.v";

  // export ift enhanced verilog design file
  design.write_verilog(exported_verilog, params);

  auto top_module = design.get_top_module();

  auto stepsize = gsl::narrow<uint32_t>(ducode::get_stepsize(vcd_data, clock));
  params["stepsize"] = stepsize;

  const VCDScope *root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));

  testbench.write_verilog(exported_testbench, params);

  spdlog::info("Simulating Exported Design + Testbench");

  auto instance = ducode::DesignInstance::create_instance(design);

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;
  auto data = ducode::simulate_design(exported_verilog, exported_testbench);
  spdlog::info("End of Simulation");
  vcd_data_vector.emplace_back(data);

  std::vector<const VCDScope *> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto &vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  //instance.write_verilog("test.v");
  //instance.write_graphviz("graph.dot");

  // z3 starts here
  z3::context ctx;
  z3::solver solver(ctx);
  // vector of net variables
  z3::expr_vector ev_edges(ctx);
  // enabling unsat core tracking
  z3::params p(ctx);
  p.set("unsat_core", true);
  solver.set(p);
  // maps input and output names to smt signal ids in ev_edges (id == index)
  std::unordered_map<std::string, uint32_t> signal_name_to_expr_id;
  // create smt instance

  const uint32_t unroll_factor = 3;
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 0);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 1);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 2);
  spdlog::debug("{}", solver.to_smt2());
  ducode::DesignInstance::write_smt2(solver, "smt_spi_master.smt2");
  std::vector<std::string> hierarchy = {top_module.get_name()};
  auto input_names = top_module.get_input_names();
  std::vector<ducode::SignalIdentifier> input_ids;
  input_ids.reserve(input_names.size());
  for (auto &iname: input_names) {
    if (iname == design.get_clk_signal_name()) {
      continue;
    }
    input_ids.push_back({iname, hierarchy});
  }
  if (smt_debug) {
    CHECK(validation_breadth_first_search(instance, ev_edges, solver, ctx, input_names, signal_name_to_expr_id,
                                          unroll_factor));
  }
  CHECK(smt_sim_validation(design, instance, ev_edges, solver, ctx, input_ids, signal_name_to_expr_id, unroll_factor, stepsize));
}

TEST_CASE("simulation_to_smt_validation_SPI_Slave", "[sim-smt_validation][advanced][.]") {
  nlohmann::json params;
  params["ift"] = false;
  bool smt_debug = false;

  auto test_parameters = GENERATE(
      std::make_tuple("SPI_Slave", "SPI_Slave", "slaveSCLK"));

  const auto &[path, design_name, clock] = test_parameters;

  //setup verilog simulation file paths
  auto verilog_file = boost::filesystem::path(TESTFILES_DIR) / path / fmt::format("{}.v", design_name);
  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / path / fmt::format("{}_tb.v", design_name);
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / path / fmt::format("{}.json", design_name);

  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  params["clock"] = clock;
  const uint32_t tag_size = 512;
  params["tag_size"] = tag_size;
  // read in json netlist
  auto design = ducode::Design::parse_json(json_file);
  design.set_clk_signal_name(clock);
  design.set_top_module_name("SPI_Slave");

  // simulate design with original verilog + testbench
  auto vcd_data = ducode::simulate_design(verilog_file, testbench_file);

  // setup exported verilog file paths
  auto exported_verilog = temp_dir / "smt_design.v";
  auto exported_testbench = temp_dir / "smt_testbench.v";

  // export ift enhanced verilog design file
  design.write_verilog(exported_verilog, params);

  auto top_module = design.get_top_module();

  auto stepsize = gsl::narrow<uint32_t>(ducode::get_stepsize(vcd_data, clock));
  params["stepsize"] = stepsize;

  const VCDScope *root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));

  testbench.write_verilog(exported_testbench, params);

  spdlog::info("Simulating Exported Design + Testbench");

  auto instance = ducode::DesignInstance::create_instance(design);

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;
  auto data = ducode::simulate_design(exported_verilog, exported_testbench);
  spdlog::info("End of Simulation");
  vcd_data_vector.emplace_back(data);

  std::vector<const VCDScope *> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto &vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  //instance.write_verilog("test.v");
  //instance.write_graphviz("graph.dot");

  // z3 starts here
  z3::context ctx;
  z3::solver solver(ctx);
  // vector of net variables
  z3::expr_vector ev_edges(ctx);
  // enabling unsat core tracking
  z3::params p(ctx);
  p.set("unsat_core", true);
  solver.set(p);
  // maps input and output names to smt signal ids in ev_edges (id == index)
  std::unordered_map<std::string, uint32_t> signal_name_to_expr_id;
  // create smt instance

  const uint32_t unroll_factor = 3;
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 0);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 1);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 2);
  spdlog::debug("{}", solver.to_smt2());
  ducode::DesignInstance::write_smt2(solver, "smt_spi_master.smt2");
  std::vector<std::string> hierarchy = {top_module.get_name()};
  auto input_names = top_module.get_input_names();
  std::vector<ducode::SignalIdentifier> input_ids;
  input_ids.reserve(input_names.size());
  for (auto &iname: input_names) {
    if (iname == design.get_clk_signal_name()) {
      continue;
    }
    input_ids.push_back({iname, hierarchy});
  }
  if (smt_debug) {
    CHECK(validation_breadth_first_search(instance, ev_edges, solver, ctx, input_names, signal_name_to_expr_id,
                                          unroll_factor));
  }
  CHECK(smt_sim_validation(design, instance, ev_edges, solver, ctx, input_ids, signal_name_to_expr_id, unroll_factor, stepsize));
}

TEST_CASE("simulation_to_smt_validation_aes", "[sim-smt_validation][advanced][.]") {
  nlohmann::json params;
  params["ift"] = false;
  bool smt_debug = false;

  auto test_parameters = GENERATE(
      std::make_tuple("aes_128", "aes_128", "clk"));

  const auto &[path, design_name, clock] = test_parameters;

  //setup verilog simulation file paths
  auto verilog_file = boost::filesystem::path(TESTFILES_DIR) / path / fmt::format("{}.v", design_name);
  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / path / fmt::format("{}_tb.v", design_name);
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / path / fmt::format("{}.json", design_name);

  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  params["clock"] = clock;
  const uint32_t tag_size = 512;
  params["tag_size"] = tag_size;
  // read in json netlist
  auto design = ducode::Design::parse_json(json_file);
  design.set_clk_signal_name("clk");
  design.set_top_module_name("aes_128");

  // simulate design with original verilog + testbench
  auto vcd_data = ducode::simulate_design(verilog_file, testbench_file);

  // setup exported verilog file paths
  auto exported_verilog = temp_dir / "smt_design.v";
  auto exported_testbench = temp_dir / "smt_testbench.v";

  // export ift enhanced verilog design file
  design.write_verilog(exported_verilog, params);

  auto top_module = design.get_top_module();

  auto stepsize = gsl::narrow<uint32_t>(ducode::get_stepsize(vcd_data, clock));
  params["stepsize"] = stepsize;

  const VCDScope *root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));

  testbench.write_verilog(exported_testbench, params);

  spdlog::info("Simulating Exported Design + Testbench");

  auto instance = ducode::DesignInstance::create_instance(design);

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;
  auto data = ducode::simulate_design(exported_verilog, exported_testbench);
  spdlog::info("End of Simulation");
  vcd_data_vector.emplace_back(data);

  std::vector<const VCDScope *> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto &vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  //instance.write_verilog("test.v");
  //instance.write_graphviz("aes.dot");

  // z3 starts here
  z3::context ctx;
  z3::solver solver(ctx);
  // vector of net variables
  z3::expr_vector ev_edges(ctx);
  // enabling unsat core tracking
  z3::params p(ctx);
  p.set("unsat_core", true);
  solver.set(p);
  // maps input and output names to smt signal ids in ev_edges (id == index)
  std::unordered_map<std::string, uint32_t> signal_name_to_expr_id;
  // create smt instance

  const uint32_t unroll_factor = 3;
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 0);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 1);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 2);
  // spdlog::debug("{}", solver.to_smt2());
  ducode::DesignInstance::write_smt2(solver, "smt_aes.smt2");
  std::vector<std::string> hierarchy = {top_module.get_name()};
  auto input_names = top_module.get_input_names();
  std::vector<ducode::SignalIdentifier> input_ids;
  input_ids.reserve(input_names.size());
  for (auto &iname: input_names) {
    if (iname == design.get_clk_signal_name()) {
      continue;
    }
    input_ids.push_back({iname, hierarchy});
  }
  if (smt_debug) {
    CHECK(validation_breadth_first_search(instance, ev_edges, solver, ctx, input_names, signal_name_to_expr_id,
                                          unroll_factor));
  }
  CHECK(smt_sim_validation(design, instance, ev_edges, solver, ctx, input_ids, signal_name_to_expr_id, unroll_factor, stepsize));
}

TEST_CASE("simulation_to_smt_validation_ethmac", "[sim-smt_validation][advanced][.]") {
  nlohmann::json params;
  params["ift"] = false;
  bool smt_debug = false;

  auto test_parameters = GENERATE(
      std::make_tuple("ethmac", "ethmac", "wb_clk_i"));

  const auto &[path, design_name, clock] = test_parameters;

  //setup verilog simulation file paths
  auto verilog_file = boost::filesystem::path(TESTFILES_DIR) / path / fmt::format("{}.v", design_name);
  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / path / fmt::format("{}_tb.v", design_name);
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / path / fmt::format("{}.json", design_name);

  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  params["clock"] = clock;
  const uint32_t tag_size = 512;
  params["tag_size"] = tag_size;
  // read in json netlist
  auto design = ducode::Design::parse_json(json_file);
  design.set_clk_signal_name(clock);
  design.set_top_module_name("ethmac");

  // simulate design with original verilog + testbench
  auto vcd_data = ducode::simulate_design(verilog_file, testbench_file);

  // setup exported verilog file paths
  auto exported_verilog = temp_dir / "smt_design.v";
  auto exported_testbench = temp_dir / "smt_testbench.v";

  // export ift enhanced verilog design file
  design.write_verilog(exported_verilog, params);

  auto top_module = design.get_top_module();

  auto stepsize = gsl::narrow<uint32_t>(ducode::get_stepsize(vcd_data, clock));
  params["stepsize"] = stepsize;

  const VCDScope *root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));

  testbench.write_verilog(exported_testbench, params);

  spdlog::info("Simulating Exported Design + Testbench");

  auto instance = ducode::DesignInstance::create_instance(design);

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;
  auto data = ducode::simulate_design(exported_verilog, exported_testbench);
  spdlog::info("End of Simulation");
  vcd_data_vector.emplace_back(data);

  std::vector<const VCDScope *> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto &vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  //instance.write_verilog("test.v");
  //instance.write_graphviz("ethmac.dot");

  // z3 starts here
  z3::context ctx;
  z3::solver solver(ctx);
  // vector of net variables
  z3::expr_vector ev_edges(ctx);
  // enabling unsat core tracking
  z3::params p(ctx);
  p.set("unsat_core", true);
  solver.set(p);
  // maps input and output names to smt signal ids in ev_edges (id == index)
  std::unordered_map<std::string, uint32_t> signal_name_to_expr_id;
  // create smt instance

  const uint32_t unroll_factor = 3;
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 0);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 1);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 2);
  // spdlog::debug("{}", solver.to_smt2());
  ducode::DesignInstance::write_smt2(solver, "smt_ethmac.smt2");
  std::vector<std::string> hierarchy = {top_module.get_name()};
  auto input_names = top_module.get_input_names();
  std::vector<ducode::SignalIdentifier> input_ids;
  input_ids.reserve(input_names.size());
  for (auto &iname: input_names) {
    if (iname == design.get_clk_signal_name()) {
      continue;
    }
    input_ids.push_back({iname, hierarchy});
  }
  if (smt_debug) {
    CHECK(validation_breadth_first_search(instance, ev_edges, solver, ctx, input_names, signal_name_to_expr_id,
                                          unroll_factor));
  }
  CHECK(smt_sim_validation(design, instance, ev_edges, solver, ctx, input_ids, signal_name_to_expr_id, unroll_factor, stepsize));
}

TEST_CASE("simulation_to_smt_validation_i2c_controller", "[sim-smt_validation][advanced][.]") {
  nlohmann::json params;
  params["ift"] = false;
  bool smt_debug = false;

  auto test_parameters = GENERATE(
      std::make_tuple("i2c_controller", "i2c_controller", "clk"));

  const auto &[path, design_name, clock] = test_parameters;

  //setup verilog simulation file paths
  auto verilog_file = boost::filesystem::path(TESTFILES_DIR) / path / fmt::format("{}.v", design_name);
  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / path / fmt::format("{}_tb.v", design_name);
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / path / fmt::format("{}.json", design_name);

  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  params["clock"] = clock;
  const uint32_t tag_size = 512;
  params["tag_size"] = tag_size;
  // read in json netlist
  auto design = ducode::Design::parse_json(json_file);
  design.set_clk_signal_name("clk");
  design.set_top_module_name("i2c_controller");

  // simulate design with original verilog + testbench
  auto vcd_data = ducode::simulate_design(verilog_file, testbench_file);

  // setup exported verilog file paths
  auto exported_verilog = temp_dir / "smt_design.v";
  auto exported_testbench = temp_dir / "smt_testbench.v";

  // export ift enhanced verilog design file
  design.write_verilog(exported_verilog, params);

  auto top_module = design.get_top_module();

  auto stepsize = gsl::narrow<uint32_t>(ducode::get_stepsize(vcd_data, clock));
  params["stepsize"] = stepsize;

  const VCDScope *root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));

  testbench.write_verilog(exported_testbench, params);

  spdlog::info("Simulating Exported Design + Testbench");

  auto instance = ducode::DesignInstance::create_instance(design);

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;
  auto data = ducode::simulate_design(exported_verilog, exported_testbench);
  spdlog::info("End of Simulation");
  vcd_data_vector.emplace_back(data);

  std::vector<const VCDScope *> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto &vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  //instance.write_verilog("test.v");
  //instance.write_graphviz("i2c_controller.dot");

  // z3 starts here
  z3::context ctx;
  z3::solver solver(ctx);
  // vector of net variables
  z3::expr_vector ev_edges(ctx);
  // enabling unsat core tracking
  z3::params p(ctx);
  p.set("unsat_core", true);
  solver.set(p);
  // maps input and output names to smt signal ids in ev_edges (id == index)
  std::unordered_map<std::string, uint32_t> signal_name_to_expr_id;
  // create smt instance

  const uint32_t unroll_factor = 3;
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 0);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 1);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 2);
  // spdlog::debug("{}", solver.to_smt2());
  ducode::DesignInstance::write_smt2(solver, "smt_i2c_controller.smt2");
  std::vector<std::string> hierarchy = {top_module.get_name()};
  auto input_names = top_module.get_input_names();
  std::vector<ducode::SignalIdentifier> input_ids;
  input_ids.reserve(input_names.size());
  for (auto &iname: input_names) {
    if (iname == design.get_clk_signal_name()) {
      continue;
    }
    input_ids.push_back({iname, hierarchy});
  }
  if (smt_debug) {
    CHECK(validation_breadth_first_search(instance, ev_edges, solver, ctx, input_names, signal_name_to_expr_id,
                                          unroll_factor));
  }
  CHECK(smt_sim_validation(design, instance, ev_edges, solver, ctx, input_ids, signal_name_to_expr_id, unroll_factor, stepsize));
}

TEST_CASE("simulation_to_smt_validation_mips_16_core_top", "[sim-smt_validation][advanced][.]") {
  nlohmann::json params;
  params["ift"] = false;
  bool smt_debug = false;

  auto test_parameters = GENERATE(
      std::make_tuple("mips_16_core_top", "mips_16_core_top", "clk"));

  const auto &[path, design_name, clock] = test_parameters;

  auto json_file = boost::filesystem::path{TESTFILES_DIR} / path / fmt::format("{}.json", design_name);

  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  params["clock"] = clock;
  const uint32_t tag_size = 512;
  params["tag_size"] = tag_size;
  // read in json netlist
  auto design = ducode::Design::parse_json(json_file);
  design.set_clk_signal_name("clk");
  design.set_top_module_name("mips_16_core_top");

  auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / path / fmt::format("{}.vcd", design_name);
  REQUIRE(boost::filesystem::exists(vcd_reference_file));
  VCDFileParser parser;
  auto vcd_data = parser.parse_file(vcd_reference_file.string());

  // setup exported verilog file paths
  auto exported_verilog = temp_dir / "smt_design.v";
  auto exported_testbench = temp_dir / "smt_testbench.v";

  // export ift enhanced verilog design file
  design.write_verilog(exported_verilog, params);

  auto top_module = design.get_top_module();

  auto stepsize = gsl::narrow<uint32_t>(ducode::get_stepsize(vcd_data, clock));
  params["stepsize"] = stepsize;

  const VCDScope *root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));

  testbench.write_verilog(exported_testbench, params);

  spdlog::info("Simulating Exported Design + Testbench");

  auto instance = ducode::DesignInstance::create_instance(design);

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;
  auto data = ducode::simulate_design(exported_verilog, exported_testbench);
  spdlog::info("End of Simulation");
  vcd_data_vector.emplace_back(data);

  std::vector<const VCDScope *> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto &vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  //instance.write_verilog("test.v");
  //instance.write_graphviz("mips_16_core_top.dot");

  // z3 starts here
  z3::context ctx;
  z3::solver solver(ctx);
  // vector of net variables
  z3::expr_vector ev_edges(ctx);
  // enabling unsat core tracking
  z3::params p(ctx);
  p.set("unsat_core", true);
  solver.set(p);
  // maps input and output names to smt signal ids in ev_edges (id == index)
  std::unordered_map<std::string, uint32_t> signal_name_to_expr_id;
  // create smt instance

  const uint32_t unroll_factor = 3;
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 0);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 1);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 2);
  // spdlog::debug("{}", solver.to_smt2());
  ducode::DesignInstance::write_smt2(solver, "smt_mips_16_core_top.smt2");
  std::vector<std::string> hierarchy = {top_module.get_name()};
  auto input_names = top_module.get_input_names();
  std::vector<ducode::SignalIdentifier> input_ids;
  input_ids.reserve(input_names.size());
  for (auto &iname: input_names) {
    if (iname == design.get_clk_signal_name()) {
      continue;
    }
    input_ids.push_back({iname, hierarchy});
  }
  if (smt_debug) {
    CHECK(validation_breadth_first_search(instance, ev_edges, solver, ctx, input_names, signal_name_to_expr_id,
                                          unroll_factor));
  }
  CHECK(smt_sim_validation(design, instance, ev_edges, solver, ctx, input_ids, signal_name_to_expr_id, unroll_factor, stepsize));
}

TEST_CASE("simulation_to_smt_validation_openMSP430", "[sim-smt_validation][advanced][.]") {
  nlohmann::json params;
  params["ift"] = false;
  bool smt_debug = false;

  auto test_parameters = GENERATE(
      std::make_tuple("openMSP430", "openMSP430", "clk"));

  const auto &[path, design_name, clock] = test_parameters;

  //setup verilog simulation file paths
  auto verilog_file = boost::filesystem::path(TESTFILES_DIR) / path / fmt::format("{}.v", design_name);
  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / path / fmt::format("{}_tb.v", design_name);
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / path / fmt::format("{}.json", design_name);

  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  params["clock"] = clock;
  const uint32_t tag_size = 512;
  params["tag_size"] = tag_size;
  // read in json netlist
  auto design = ducode::Design::parse_json(json_file);
  design.set_clk_signal_name("clk");
  design.set_top_module_name("openMSP430");

  // simulate design with original verilog + testbench
  auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / path / fmt::format("{}.vcd", design_name);
  REQUIRE(boost::filesystem::exists(vcd_reference_file));
  VCDFileParser parser;
  auto vcd_data = parser.parse_file(vcd_reference_file.string());

  // setup exported verilog file paths
  auto exported_verilog = temp_dir / "smt_design.v";
  auto exported_testbench = temp_dir / "smt_testbench.v";

  // export ift enhanced verilog design file
  design.write_verilog(exported_verilog, params);

  auto top_module = design.get_top_module();

  auto stepsize = gsl::narrow<uint32_t>(ducode::get_stepsize(vcd_data, clock));
  params["stepsize"] = stepsize;

  const VCDScope *root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));

  testbench.write_verilog(exported_testbench, params);

  spdlog::info("Simulating Exported Design + Testbench");

  auto instance = ducode::DesignInstance::create_instance(design);

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;
  auto data = ducode::simulate_design(exported_verilog, exported_testbench);
  spdlog::info("End of Simulation");
  vcd_data_vector.emplace_back(data);

  std::vector<const VCDScope *> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto &vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  //instance.write_verilog("test.v");
  //instance.write_graphviz("openMSP430.dot");

  // z3 starts here
  z3::context ctx;
  z3::solver solver(ctx);
  // vector of net variables
  z3::expr_vector ev_edges(ctx);
  // enabling unsat core tracking
  z3::params p(ctx);
  p.set("unsat_core", true);
  solver.set(p);
  // maps input and output names to smt signal ids in ev_edges (id == index)
  std::unordered_map<std::string, uint32_t> signal_name_to_expr_id;
  // create smt instance

  const uint32_t unroll_factor = 3;
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 0);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 1);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 2);
  // spdlog::debug("{}", solver.to_smt2());
  ducode::DesignInstance::write_smt2(solver, "smt_openMSP430.smt2");
  std::vector<std::string> hierarchy = {top_module.get_name()};
  auto input_names = top_module.get_input_names();
  std::vector<ducode::SignalIdentifier> input_ids;
  input_ids.reserve(input_names.size());
  for (auto &iname: input_names) {
    if (iname == design.get_clk_signal_name()) {
      continue;
    }
    input_ids.push_back({iname, hierarchy});
  }
  if (smt_debug) {
    CHECK(validation_breadth_first_search(instance, ev_edges, solver, ctx, input_names, signal_name_to_expr_id,
                                          unroll_factor));
  }
  CHECK(smt_sim_validation(design, instance, ev_edges, solver, ctx, input_ids, signal_name_to_expr_id, unroll_factor, stepsize));
}

TEST_CASE("simulation_to_smt_validation_picorv32", "[sim-smt_validation][advanced][.]") {
  nlohmann::json params;
  params["ift"] = false;
  bool smt_debug = false;

  auto test_parameters = GENERATE(
      std::make_tuple("picorv32", "picorv32", "clk"));

  const auto &[path, design_name, clock] = test_parameters;

  //setup verilog simulation file paths
  auto verilog_file = boost::filesystem::path(TESTFILES_DIR) / path / fmt::format("{}.v", design_name);
  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / path / fmt::format("{}_tb.v", design_name);
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / path / fmt::format("{}.json", design_name);

  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  params["clock"] = clock;
  const uint32_t tag_size = 512;
  params["tag_size"] = tag_size;
  // read in json netlist
  auto design = ducode::Design::parse_json(json_file);
  design.set_clk_signal_name("clk");
  design.set_top_module_name("picorv32");

  // simulate design with original verilog + testbench
  auto vcd_data = ducode::simulate_design(verilog_file, testbench_file);

  // setup exported verilog file paths
  auto exported_verilog = temp_dir / "smt_design.v";
  auto exported_testbench = temp_dir / "smt_testbench.v";

  // export ift enhanced verilog design file
  design.write_verilog(exported_verilog, params);

  auto top_module = design.get_top_module();

  auto stepsize = gsl::narrow<uint32_t>(ducode::get_stepsize(vcd_data, clock));
  params["stepsize"] = stepsize;

  const VCDScope *root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));

  testbench.write_verilog(exported_testbench, params);

  spdlog::info("Simulating Exported Design + Testbench");

  auto instance = ducode::DesignInstance::create_instance(design);

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;
  auto data = ducode::simulate_design(exported_verilog, exported_testbench);
  spdlog::info("End of Simulation");
  vcd_data_vector.emplace_back(data);

  std::vector<const VCDScope *> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto &vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  //instance.write_verilog("test.v");
  //instance.write_graphviz("picorv32.dot");

  // z3 starts here
  z3::context ctx;
  z3::solver solver(ctx);
  // vector of net variables
  z3::expr_vector ev_edges(ctx);
  // enabling unsat core tracking
  z3::params p(ctx);
  p.set("unsat_core", true);
  solver.set(p);
  // maps input and output names to smt signal ids in ev_edges (id == index)
  std::unordered_map<std::string, uint32_t> signal_name_to_expr_id;
  // create smt instance

  const uint32_t unroll_factor = 3;
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 0);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 1);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 2);
  // spdlog::debug("{}", solver.to_smt2());
  ducode::DesignInstance::write_smt2(solver, "smt_picorv32.smt2");
  std::vector<std::string> hierarchy = {top_module.get_name()};
  auto input_names = top_module.get_input_names();
  std::vector<ducode::SignalIdentifier> input_ids;
  input_ids.reserve(input_names.size());
  for (auto &iname: input_names) {
    if (iname == design.get_clk_signal_name()) {
      continue;
    }
    input_ids.push_back({iname, hierarchy});
  }
  if (smt_debug) {
    CHECK(validation_breadth_first_search(instance, ev_edges, solver, ctx, input_names, signal_name_to_expr_id,
                                          unroll_factor));
  }
  CHECK(smt_sim_validation(design, instance, ev_edges, solver, ctx, input_ids, signal_name_to_expr_id, unroll_factor, stepsize));
}

TEST_CASE("simulation_to_smt_validation_usb2uart", "[sim-smt_validation][advanced][.]") {
  nlohmann::json params;
  params["ift"] = false;
  bool smt_debug = false;

  auto test_parameters = GENERATE(
      std::make_tuple("usb2uart", "usb2uart", "clk"));

  const auto &[path, design_name, clock] = test_parameters;

  //setup verilog simulation file paths
  auto verilog_file = boost::filesystem::path(TESTFILES_DIR) / path / fmt::format("core.v", design_name);
  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / path / fmt::format("{}_tb.v", design_name);
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / path / fmt::format("{}.json", design_name);

  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  params["clock"] = clock;
  const uint32_t tag_size = 512;
  params["tag_size"] = tag_size;
  // read in json netlist
  auto design = ducode::Design::parse_json(json_file);
  design.set_clk_signal_name("clk");
  design.set_top_module_name("core");

  // simulate design with original verilog + testbench
  auto vcd_data = ducode::simulate_design(verilog_file, testbench_file);

  // setup exported verilog file paths
  auto exported_verilog = temp_dir / "smt_design.v";
  auto exported_testbench = temp_dir / "smt_testbench.v";

  // export ift enhanced verilog design file
  design.write_verilog(exported_verilog, params);

  auto top_module = design.get_top_module();

  auto stepsize = gsl::narrow<uint32_t>(ducode::get_stepsize(vcd_data, clock));
  params["stepsize"] = stepsize;

  const VCDScope *root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));

  testbench.write_verilog(exported_testbench, params);

  spdlog::info("Simulating Exported Design + Testbench");

  auto instance = ducode::DesignInstance::create_instance(design);

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;
  auto data = ducode::simulate_design(exported_verilog, exported_testbench);
  spdlog::info("End of Simulation");
  vcd_data_vector.emplace_back(data);

  std::vector<const VCDScope *> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto &vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  //instance.write_verilog("test.v");
  //instance.write_graphviz("usb2uart.dot");

  // z3 starts here
  z3::context ctx;
  z3::solver solver(ctx);
  // vector of net variables
  z3::expr_vector ev_edges(ctx);
  // enabling unsat core tracking
  z3::params p(ctx);
  p.set("unsat_core", true);
  solver.set(p);
  // maps input and output names to smt signal ids in ev_edges (id == index)
  std::unordered_map<std::string, uint32_t> signal_name_to_expr_id;
  // create smt instance

  const uint32_t unroll_factor = 3;
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 0);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 1);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 2);
  // spdlog::debug("{}", solver.to_smt2());
  ducode::DesignInstance::write_smt2(solver, "smt_usb2uart.smt2");
  std::vector<std::string> hierarchy = {top_module.get_name()};
  auto input_names = top_module.get_input_names();
  std::vector<ducode::SignalIdentifier> input_ids;
  input_ids.reserve(input_names.size());
  for (auto &iname: input_names) {
    if (iname == design.get_clk_signal_name()) {
      continue;
    }
    input_ids.push_back({iname, hierarchy});
  }
  if (smt_debug) {
    CHECK(validation_breadth_first_search(instance, ev_edges, solver, ctx, input_names, signal_name_to_expr_id,
                                          unroll_factor));
  }
  CHECK(smt_sim_validation(design, instance, ev_edges, solver, ctx, input_ids, signal_name_to_expr_id, unroll_factor, stepsize));
}

TEST_CASE("simulation_to_smt_validation_yadmc", "[sim-smt_validation][advanced][.]") {
  nlohmann::json params;
  params["ift"] = false;
  bool smt_debug = false;

  auto test_parameters = GENERATE(
      std::make_tuple("yadmc", "yadmc", "clk"));

  const auto &[path, design_name, clock] = test_parameters;

  //setup verilog simulation file path
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / path / fmt::format("{}.json", design_name);

  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  params["clock"] = clock;
  const uint32_t tag_size = 512;
  params["tag_size"] = tag_size;
  // read in json netlist
  auto design = ducode::Design::parse_json(json_file);
  design.set_clk_signal_name("clk");
  design.set_top_module_name("yadmc");

  auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / path / fmt::format("{}.vcd", design_name);
  REQUIRE(boost::filesystem::exists(vcd_reference_file));
  VCDFileParser parser;
  auto vcd_data = parser.parse_file(vcd_reference_file.string());

  // setup exported verilog file paths
  auto exported_verilog = temp_dir / "smt_design.v";
  auto exported_testbench = temp_dir / "smt_testbench.v";

  // export ift enhanced verilog design file
  design.write_verilog(exported_verilog, params);

  auto top_module = design.get_top_module();

  auto stepsize = gsl::narrow<uint32_t>(ducode::get_stepsize(vcd_data, clock));
  params["stepsize"] = stepsize;

  const VCDScope *root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));

  testbench.write_verilog(exported_testbench, params);

  spdlog::info("Simulating Exported Design + Testbench");

  auto instance = ducode::DesignInstance::create_instance(design);

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;
  auto data = ducode::simulate_design(exported_verilog, exported_testbench);
  spdlog::info("End of Simulation");
  vcd_data_vector.emplace_back(data);

  std::vector<const VCDScope *> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto &vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  //instance.write_verilog("test.v");
  //instance.write_graphviz("yadmc.dot");

  // z3 starts here
  z3::context ctx;
  z3::solver solver(ctx);
  // vector of net variables
  z3::expr_vector ev_edges(ctx);
  // enabling unsat core tracking
  z3::params p(ctx);
  p.set("unsat_core", true);
  solver.set(p);
  // maps input and output names to smt signal ids in ev_edges (id == index)
  std::unordered_map<std::string, uint32_t> signal_name_to_expr_id;
  // create smt instance

  const uint32_t unroll_factor = 3;
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 0);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 1);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_expr_id, 1, 2);
  // spdlog::debug("{}", solver.to_smt2());
  ducode::DesignInstance::write_smt2(solver, "smt_yadmc.smt2");
  std::vector<std::string> hierarchy = {top_module.get_name()};
  auto input_names = top_module.get_input_names();
  std::vector<ducode::SignalIdentifier> input_ids;
  input_ids.reserve(input_names.size());
  for (auto &iname: input_names) {
    if (iname == design.get_clk_signal_name()) {
      continue;
    }
    input_ids.push_back({iname, hierarchy});
  }
  if (smt_debug) {
    CHECK(validation_breadth_first_search(instance, ev_edges, solver, ctx, input_names, signal_name_to_expr_id,
                                          unroll_factor));
  }
  CHECK(smt_sim_validation(design, instance, ev_edges, solver, ctx, input_ids, signal_name_to_expr_id, unroll_factor, stepsize));
}

TEST_CASE("smt_to_simulation_validation_dff_hierarchy_design", "[smt-sim_validation][basic][.]") {
  nlohmann::json params;
  params["ift"] = false;
  bool smt_debug = false;

  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "dff_hierarchy_design" / "dff_hierarchy_design.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  params["clock"] = "clk";
  const uint32_t tag_size = 512;
  params["tag_size"] = tag_size;
  // read in json netlist
  design.set_clk_signal_name("clk");
  design.set_top_module_name("dff_hierarchy_design");

  //instance.write_verilog("test.v");
  //instance.write_graphviz("graph_unrolling_dff_hierarchy_design.dot");

  // call design verilog export
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  // setup exported verilog file paths
  auto exported_verilog = temp_dir / "smt_validation_design.v";
  auto exported_testbench = temp_dir / "smt_validation_testbench.v";

  // export ift enhanced verilog design file
  design.write_verilog(exported_verilog, params);

  // z3 test starts here
  auto ctx = std::make_shared<z3::context>();
  auto solver = std::make_shared<z3::solver>(*ctx);
  auto ev_edges = std::make_shared<z3::expr_vector>(*ctx);
  uint32_t unroll_factor = 4;
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 0);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 1);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 2);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 3);

  REQUIRE(solver->check() == z3::sat);

  std::vector<std::string> hierarchy = {design.get_top_module().get_name()};
  auto input_names = design.get_top_module().get_input_names();
  std::vector<ducode::SignalIdentifier> input_ids;
  input_ids.reserve(input_names.size());
  for (auto &iname: input_names) {
    if (iname == design.get_clk_signal_name()) {
      continue;
    }
    input_ids.push_back({iname, hierarchy});
  }

  auto output_names = design.get_top_module().get_output_names();
  std::vector<ducode::SignalIdentifier> output_ids;
  output_ids.reserve(output_names.size());
  for (auto &oname: output_names) {
    output_ids.push_back({oname, hierarchy});
  }
  auto testbench_value_map = std::make_shared<ducode::SMTSignalsDataManager>(ctx, solver, signal_name_to_edge_id, unroll_factor, ev_edges, input_ids, output_ids, design.get_top_module_name());
  ducode::TestbenchSMT testbench(design, testbench_value_map, instance);

  testbench.write_verilog(exported_testbench, params);

  z3::model s_model = solver->get_model();

  // generating an assignment for a verilog model using SMT, then writing the inputs and initial state values into a testbench, comparing the output values for the timesteps after the initial state between SMT model and verilog simulation trace

  spdlog::info("Simulating Exported Design + Testbench");

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;
  auto data = ducode::simulate_design(exported_verilog, exported_testbench);
  spdlog::info("End of Simulation");
  vcd_data_vector.emplace_back(data);

  std::vector<const VCDScope *> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto &vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  CHECK(smt_to_simulation_validation(instance, ev_edges, solver, signal_name_to_edge_id, unroll_factor));

  // call simulation
}

TEST_CASE("smt_to_simulation_validation_y86", "[smt-sim_validation][advanced][.]") {
  nlohmann::json params;
  params["ift"] = false;
  bool smt_debug = false;

  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "y86" / "y86.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  params["clock"] = "clk";
  const uint32_t tag_size = 512;
  params["tag_size"] = tag_size;
  // read in json netlist
  design.set_clk_signal_name("clk");
  design.set_top_module_name("cisc_cpu_p");

  //instance.write_verilog("test.v");
  //instance.write_graphviz("graph_unrolling_y86.dot");

  // call design verilog export
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  // setup exported verilog file paths
  auto exported_verilog = temp_dir / "smt_validation_design.v";
  auto exported_testbench = temp_dir / "smt_validation_testbench.v";

  // export ift enhanced verilog design file
  design.write_verilog(exported_verilog, params);

  // z3 test starts here
  auto ctx = std::make_shared<z3::context>();
  auto solver = std::make_shared<z3::solver>(*ctx);
  auto ev_edges = std::make_shared<z3::expr_vector>(*ctx);
  uint32_t unroll_factor = 4;
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 0);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 1);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 2);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 3);

  REQUIRE(solver->check() == z3::sat);

  std::vector<std::string> hierarchy = {design.get_top_module().get_name()};
  auto input_names = design.get_top_module().get_input_names();
  std::vector<ducode::SignalIdentifier> input_ids;
  input_ids.reserve(input_names.size());
  for (auto &iname: input_names) {
    if (iname == design.get_clk_signal_name()) {
      continue;
    }
    input_ids.push_back({iname, hierarchy});
  }

  auto output_names = design.get_top_module().get_output_names();
  std::vector<ducode::SignalIdentifier> output_ids;
  output_ids.reserve(output_names.size());
  for (auto &oname: output_names) {
    output_ids.push_back({oname, hierarchy});
  }
  auto testbench_value_map = std::make_shared<ducode::SMTSignalsDataManager>(ctx, solver, signal_name_to_edge_id, unroll_factor, ev_edges, input_ids, output_ids, design.get_top_module_name());
  ducode::TestbenchSMT testbench(design, testbench_value_map, instance);

  testbench.write_verilog(exported_testbench, params);

  z3::model s_model = solver->get_model();

  // generating an assignment for a verilog model using SMT, then writing the inputs and initial state values into a testbench, comparing the output values for the timesteps after the initial state between SMT model and verilog simulation trace

  spdlog::info("Simulating Exported Design + Testbench");

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;
  auto data = ducode::simulate_design(exported_verilog, exported_testbench);
  spdlog::info("End of Simulation");
  vcd_data_vector.emplace_back(data);

  std::vector<const VCDScope *> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto &vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  CHECK(smt_to_simulation_validation(instance, ev_edges, solver, signal_name_to_edge_id, unroll_factor));

  // call simulation
}

TEST_CASE("smt_to_simulation_validation_SPI_Master", "[smt-sim_validation][advanced][.]") {
  nlohmann::json params;
  params["ift"] = false;
  bool smt_debug = false;

  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "SPI_Master" / "SPI_Master.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  params["clock"] = "clk";
  const uint32_t tag_size = 512;
  params["tag_size"] = tag_size;
  // read in json netlist
  design.set_clk_signal_name("clk");
  design.set_top_module_name("SPI_Master");

  //instance.write_verilog("test.v");
  //instance.write_graphviz("graph_unrolling_y86.dot");

  // call design verilog export
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  // setup exported verilog file paths
  auto exported_verilog = temp_dir / "smt_validation_design.v";
  auto exported_testbench = temp_dir / "smt_validation_testbench.v";

  // export ift enhanced verilog design file
  design.write_verilog(exported_verilog, params);

  // z3 test starts here
  auto ctx = std::make_shared<z3::context>();
  auto solver = std::make_shared<z3::solver>(*ctx);
  auto ev_edges = std::make_shared<z3::expr_vector>(*ctx);
  uint32_t unroll_factor = 4;
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 0);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 1);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 2);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 3);

  REQUIRE(solver->check() == z3::sat);

  std::vector<std::string> hierarchy = {design.get_top_module().get_name()};
  auto input_names = design.get_top_module().get_input_names();
  std::vector<ducode::SignalIdentifier> input_ids;
  input_ids.reserve(input_names.size());
  for (auto &iname: input_names) {
    if (iname == design.get_clk_signal_name()) {
      continue;
    }
    input_ids.push_back({iname, hierarchy});
  }

  auto output_names = design.get_top_module().get_output_names();
  std::vector<ducode::SignalIdentifier> output_ids;
  output_ids.reserve(output_names.size());
  for (auto &oname: output_names) {
    output_ids.push_back({oname, hierarchy});
  }
  auto testbench_value_map = std::make_shared<ducode::SMTSignalsDataManager>(ctx, solver, signal_name_to_edge_id, unroll_factor, ev_edges, input_ids, output_ids, design.get_top_module_name());
  ducode::TestbenchSMT testbench(design, testbench_value_map, instance);

  testbench.write_verilog(exported_testbench, params);

  z3::model s_model = solver->get_model();

  // generating an assignment for a verilog model using SMT, then writing the inputs and initial state values into a testbench, comparing the output values for the timesteps after the initial state between SMT model and verilog simulation trace

  spdlog::info("Simulating Exported Design + Testbench");

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;
  auto data = ducode::simulate_design(exported_verilog, exported_testbench);
  spdlog::info("End of Simulation");
  vcd_data_vector.emplace_back(data);

  std::vector<const VCDScope *> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto &vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  CHECK(smt_to_simulation_validation(instance, ev_edges, solver, signal_name_to_edge_id, unroll_factor));

  // call simulation
}

TEST_CASE("smt_to_simulation_validation_picorv32", "[smt-sim_validation][advanced][.]") {
  nlohmann::json params;
  params["ift"] = false;
  bool smt_debug = false;

  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "picorv32" / "picorv32.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  params["clock"] = "clk";
  const uint32_t tag_size = 512;
  params["tag_size"] = tag_size;
  // read in json netlist
  design.set_clk_signal_name("clk");
  design.set_top_module_name("picorv32");

  //instance.write_verilog("test.v");
  //instance.write_graphviz("graph_unrolling_picorv32.dot");

  // call design verilog export
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  // setup exported verilog file paths
  auto exported_verilog = temp_dir / "smt_validation_design.v";
  auto exported_testbench = temp_dir / "smt_validation_testbench.v";

  // export ift enhanced verilog design file
  design.write_verilog(exported_verilog, params);

  // z3 test starts here
  auto ctx = std::make_shared<z3::context>();
  auto solver = std::make_shared<z3::solver>(*ctx);
  auto ev_edges = std::make_shared<z3::expr_vector>(*ctx);
  uint32_t unroll_factor = 4;
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 0);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 1);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 2);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 3);

  REQUIRE(solver->check() == z3::sat);

  std::vector<std::string> hierarchy = {design.get_top_module().get_name()};
  auto input_names = design.get_top_module().get_input_names();
  std::vector<ducode::SignalIdentifier> input_ids;
  input_ids.reserve(input_names.size());
  for (auto &iname: input_names) {
    if (iname == design.get_clk_signal_name()) {
      continue;
    }
    input_ids.push_back({iname, hierarchy});
  }

  auto output_names = design.get_top_module().get_output_names();
  std::vector<ducode::SignalIdentifier> output_ids;
  output_ids.reserve(output_names.size());
  for (auto &oname: output_names) {
    output_ids.push_back({oname, hierarchy});
  }
  auto testbench_value_map = std::make_shared<ducode::SMTSignalsDataManager>(ctx, solver, signal_name_to_edge_id, unroll_factor, ev_edges, input_ids, output_ids, design.get_top_module_name());
  ducode::TestbenchSMT testbench(design, testbench_value_map, instance);

  testbench.write_verilog(exported_testbench, params);

  z3::model s_model = solver->get_model();

  // generating an assignment for a verilog model using SMT, then writing the inputs and initial state values into a testbench, comparing the output values for the timesteps after the initial state between SMT model and verilog simulation trace

  spdlog::info("Simulating Exported Design + Testbench");

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;
  auto data = ducode::simulate_design(exported_verilog, exported_testbench);
  spdlog::info("End of Simulation");
  vcd_data_vector.emplace_back(data);

  std::vector<const VCDScope *> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto &vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  CHECK(smt_to_simulation_validation(instance, ev_edges, solver, signal_name_to_edge_id, unroll_factor));

  // call simulation
}

TEST_CASE("smt_to_simulation_validation_aes", "[smt-sim_validation][advanced][.]") {
  nlohmann::json params;
  params["ift"] = false;
  bool smt_debug = false;

  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "aes_128" / "aes_128.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  params["clock"] = "clk";
  const uint32_t tag_size = 512;
  params["tag_size"] = tag_size;
  // read in json netlist
  design.set_clk_signal_name("clk");
  design.set_top_module_name("aes_128");

  //instance.write_verilog("test.v");
  //instance.write_graphviz("graph_unrolling_aes_128.dot");

  // call design verilog export
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  // setup exported verilog file paths
  auto exported_verilog = temp_dir / "smt_validation_design.v";
  auto exported_testbench = temp_dir / "smt_validation_testbench.v";

  // export ift enhanced verilog design file
  design.write_verilog(exported_verilog, params);

  // z3 test starts here
  auto ctx = std::make_shared<z3::context>();
  auto solver = std::make_shared<z3::solver>(*ctx);
  auto ev_edges = std::make_shared<z3::expr_vector>(*ctx);
  uint32_t unroll_factor = 4;
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 0);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 1);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 2);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 3);

  REQUIRE(solver->check() == z3::sat);

  std::vector<std::string> hierarchy = {design.get_top_module().get_name()};
  auto input_names = design.get_top_module().get_input_names();
  std::vector<ducode::SignalIdentifier> input_ids;
  input_ids.reserve(input_names.size());
  for (auto &iname: input_names) {
    if (iname == design.get_clk_signal_name()) {
      continue;
    }
    input_ids.push_back({iname, hierarchy});
  }

  auto output_names = design.get_top_module().get_output_names();
  std::vector<ducode::SignalIdentifier> output_ids;
  output_ids.reserve(output_names.size());
  for (auto &oname: output_names) {
    output_ids.push_back({oname, hierarchy});
  }
  auto testbench_value_map = std::make_shared<ducode::SMTSignalsDataManager>(ctx, solver, signal_name_to_edge_id, unroll_factor, ev_edges, input_ids, output_ids, design.get_top_module_name());
  ducode::TestbenchSMT testbench(design, testbench_value_map, instance);

  testbench.write_verilog(exported_testbench, params);

  z3::model s_model = solver->get_model();

  // generating an assignment for a verilog model using SMT, then writing the inputs and initial state values into a testbench, comparing the output values for the timesteps after the initial state between SMT model and verilog simulation trace

  spdlog::info("Simulating Exported Design + Testbench");

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;
  auto data = ducode::simulate_design(exported_verilog, exported_testbench);
  spdlog::info("End of Simulation");
  vcd_data_vector.emplace_back(data);

  std::vector<const VCDScope *> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto &vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  CHECK(smt_to_simulation_validation(instance, ev_edges, solver, signal_name_to_edge_id, unroll_factor));

  // call simulation
}

TEST_CASE("smt_to_simulation_validation_openMSP430", "[smt-sim_validation][advanced][.]") {
  nlohmann::json params;
  params["ift"] = false;
  bool smt_debug = false;

  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "openMSP430" / "openMSP430.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  std::string clock = "mclk";
  params["clock"] = clock;
  const uint32_t tag_size = 512;
  params["tag_size"] = tag_size;
  // read in json netlist
  design.set_clk_signal_name(clock);
  design.set_top_module_name("openMSP430");

  //instance.write_verilog("test.v");
  //instance.write_graphviz("graph_unrolling_openMSP430.dot");

  // call design verilog export
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  // setup exported verilog file paths
  auto exported_verilog = temp_dir / "smt_validation_design.v";
  auto exported_testbench = temp_dir / "smt_validation_testbench.v";

  // export ift enhanced verilog design file
  design.write_verilog(exported_verilog, params);

  // z3 test starts here
  auto ctx = std::make_shared<z3::context>();
  auto solver = std::make_shared<z3::solver>(*ctx);
  auto ev_edges = std::make_shared<z3::expr_vector>(*ctx);
  uint32_t unroll_factor = 4;
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 0);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 1);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 2);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 3);

  REQUIRE(solver->check() == z3::sat);

  std::vector<std::string> hierarchy = {design.get_top_module().get_name()};
  auto input_names = design.get_top_module().get_input_names();
  std::vector<ducode::SignalIdentifier> input_ids;
  input_ids.reserve(input_names.size());
  for (auto &iname: input_names) {
    if (iname == design.get_clk_signal_name()) {
      continue;
    }
    input_ids.push_back({iname, hierarchy});
  }

  auto output_names = design.get_top_module().get_output_names();
  std::vector<ducode::SignalIdentifier> output_ids;
  output_ids.reserve(output_names.size());
  for (auto &oname: output_names) {
    output_ids.push_back({oname, hierarchy});
  }
  auto testbench_value_map = std::make_shared<ducode::SMTSignalsDataManager>(ctx, solver, signal_name_to_edge_id, unroll_factor, ev_edges, input_ids, output_ids, design.get_top_module_name());
  ducode::TestbenchSMT testbench(design, testbench_value_map, instance);

  testbench.write_verilog(exported_testbench, params);

  z3::model s_model = solver->get_model();

  // generating an assignment for a verilog model using SMT, then writing the inputs and initial state values into a testbench, comparing the output values for the timesteps after the initial state between SMT model and verilog simulation trace

  spdlog::info("Simulating Exported Design + Testbench");

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;
  auto data = ducode::simulate_design(exported_verilog, exported_testbench);
  spdlog::info("End of Simulation");
  vcd_data_vector.emplace_back(data);

  std::vector<const VCDScope *> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto &vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  CHECK(smt_to_simulation_validation(instance, ev_edges, solver, signal_name_to_edge_id, unroll_factor));

  // call simulation
}

TEST_CASE("smt_to_simulation_validation_ethmac", "[smt-sim_validation][advanced][.]") {
  nlohmann::json params;
  params["ift"] = false;
  bool smt_debug = false;

  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "ethmac" / "ethmac.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  params["clock"] = "clk";
  const uint32_t tag_size = 512;
  params["tag_size"] = tag_size;
  // read in json netlist
  design.set_clk_signal_name("clk");
  design.set_top_module_name("ethmac");

  //instance.write_verilog("test.v");
  //instance.write_graphviz("graph_unrolling_ethmac.dot");

  // call design verilog export
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  // setup exported verilog file paths
  auto exported_verilog = temp_dir / "smt_validation_design.v";
  auto exported_testbench = temp_dir / "smt_validation_testbench.v";

  // export ift enhanced verilog design file
  design.write_verilog(exported_verilog, params);

  // z3 test starts here
  auto ctx = std::make_shared<z3::context>();
  auto solver = std::make_shared<z3::solver>(*ctx);
  auto ev_edges = std::make_shared<z3::expr_vector>(*ctx);
  uint32_t unroll_factor = 4;
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 0);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 1);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 2);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 3);

  REQUIRE(solver->check() == z3::sat);

  std::vector<std::string> hierarchy = {design.get_top_module().get_name()};
  auto input_names = design.get_top_module().get_input_names();
  std::vector<ducode::SignalIdentifier> input_ids;
  input_ids.reserve(input_names.size());
  for (auto &iname: input_names) {
    if (iname == design.get_clk_signal_name()) {
      continue;
    }
    input_ids.push_back({iname, hierarchy});
  }

  auto output_names = design.get_top_module().get_output_names();
  std::vector<ducode::SignalIdentifier> output_ids;
  output_ids.reserve(output_names.size());
  for (auto &oname: output_names) {
    output_ids.push_back({oname, hierarchy});
  }
  auto testbench_value_map = std::make_shared<ducode::SMTSignalsDataManager>(ctx, solver, signal_name_to_edge_id, unroll_factor, ev_edges, input_ids, output_ids, design.get_top_module_name());
  ducode::TestbenchSMT testbench(design, testbench_value_map, instance);

  testbench.write_verilog(exported_testbench, params);

  z3::model s_model = solver->get_model();

  // generating an assignment for a verilog model using SMT, then writing the inputs and initial state values into a testbench, comparing the output values for the timesteps after the initial state between SMT model and verilog simulation trace

  spdlog::info("Simulating Exported Design + Testbench");

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;
  auto data = ducode::simulate_design(exported_verilog, exported_testbench);
  spdlog::info("End of Simulation");
  vcd_data_vector.emplace_back(data);

  std::vector<const VCDScope *> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto &vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  CHECK(smt_to_simulation_validation(instance, ev_edges, solver, signal_name_to_edge_id, unroll_factor));

  // call simulation
}

TEST_CASE("smt_to_simulation_validation_i2c_controller", "[smt-sim_validation][advanced][.]") {
  nlohmann::json params;
  params["ift"] = false;
  bool smt_debug = false;

  spdlog::info("1. Parse Design JSON file");
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "i2c_controller" / "i2c_controller.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  params["clock"] = "clk";
  const uint32_t tag_size = 512;
  params["tag_size"] = tag_size;
  // read in json netlist
  design.set_clk_signal_name("clk");
  design.set_top_module_name("i2c_controller");

  //instance.write_verilog("test.v");
  //instance.write_graphviz("graph_unrolling_i2c_controller.dot");

  // call design verilog export
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  // setup exported verilog file paths
  auto exported_verilog = temp_dir / "smt_validation_design.v";
  auto exported_testbench = temp_dir / "smt_validation_testbench.v";

  // export ift enhanced verilog design file
  design.write_verilog(exported_verilog, params);

  spdlog::info("2. Setup SMT solver");
  // z3 test starts here
  auto ctx = std::make_shared<z3::context>();
  auto solver = std::make_shared<z3::solver>(*ctx);
  auto ev_edges = std::make_shared<z3::expr_vector>(*ctx);
  uint32_t unroll_factor = 4;
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 0);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 1);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 2);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 3);

  REQUIRE(solver->check() == z3::sat);

  spdlog::info("3. Setup Verilog Simulation");
  std::vector<std::string> hierarchy = {design.get_top_module().get_name()};
  auto input_names = design.get_top_module().get_input_names();
  std::vector<ducode::SignalIdentifier> input_ids;
  input_ids.reserve(input_names.size());
  for (auto &iname: input_names) {
    if (iname == design.get_clk_signal_name()) {
      continue;
    }
    input_ids.push_back({iname, hierarchy});
  }

  auto output_names = design.get_top_module().get_output_names();
  std::vector<ducode::SignalIdentifier> output_ids;
  output_ids.reserve(output_names.size());
  for (auto &oname: output_names) {
    output_ids.push_back({oname, hierarchy});
  }
  auto testbench_value_map = std::make_shared<ducode::SMTSignalsDataManager>(ctx, solver, signal_name_to_edge_id, unroll_factor, ev_edges, input_ids, output_ids, design.get_top_module_name());
  ducode::TestbenchSMT testbench(design, testbench_value_map, instance);

  testbench.write_verilog(exported_testbench, params);

  z3::model s_model = solver->get_model();

  // generating an assignment for a verilog model using SMT, then writing the inputs and initial state values into a testbench, comparing the output values for the timesteps after the initial state between SMT model and verilog simulation trace

  spdlog::info("Simulating Exported Design + Testbench");

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;
  auto data = ducode::simulate_design(exported_verilog, exported_testbench);
  spdlog::info("End of Simulation");
  vcd_data_vector.emplace_back(data);

  std::vector<const VCDScope *> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto &vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  CHECK(smt_to_simulation_validation(instance, ev_edges, solver, signal_name_to_edge_id, unroll_factor));

  // call simulation
}

TEST_CASE("smt_to_simulation_validation_mips_16_core_top", "[smt-sim_validation][advanced][.]") {
  nlohmann::json params;
  params["ift"] = false;
  bool smt_debug = false;

  spdlog::info("1. Parse Design JSON file");
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "mips_16_core_top" / "mips_16_core_top.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  params["clock"] = "clk";
  const uint32_t tag_size = 512;
  params["tag_size"] = tag_size;
  // read in json netlist
  design.set_clk_signal_name("clk");
  design.set_top_module_name("mips_16_core_top");

  //instance.write_verilog("test.v");
  //instance.write_graphviz("graph_unrolling_mips_16_core_top.dot");

  // call design verilog export
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  // setup exported verilog file paths
  auto exported_verilog = temp_dir / "smt_validation_design.v";
  auto exported_testbench = temp_dir / "smt_validation_testbench.v";

  // export ift enhanced verilog design file
  design.write_verilog(exported_verilog, params);

  spdlog::info("2. Setup SMT solver");
  // z3 test starts here
  auto ctx = std::make_shared<z3::context>();
  auto solver = std::make_shared<z3::solver>(*ctx);
  auto ev_edges = std::make_shared<z3::expr_vector>(*ctx);
  uint32_t unroll_factor = 4;
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 0);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 1);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 2);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 3);

  REQUIRE(solver->check() == z3::sat);

  spdlog::info("3. Setup Verilog Simulation");
  std::vector<std::string> hierarchy = {design.get_top_module().get_name()};
  auto input_names = design.get_top_module().get_input_names();
  std::vector<ducode::SignalIdentifier> input_ids;
  input_ids.reserve(input_names.size());
  for (auto &iname: input_names) {
    if (iname == design.get_clk_signal_name()) {
      continue;
    }
    input_ids.push_back({iname, hierarchy});
  }

  auto output_names = design.get_top_module().get_output_names();
  std::vector<ducode::SignalIdentifier> output_ids;
  output_ids.reserve(output_names.size());
  for (auto &oname: output_names) {
    output_ids.push_back({oname, hierarchy});
  }
  auto testbench_value_map = std::make_shared<ducode::SMTSignalsDataManager>(ctx, solver, signal_name_to_edge_id, unroll_factor, ev_edges, input_ids, output_ids, design.get_top_module_name());
  ducode::TestbenchSMT testbench(design, testbench_value_map, instance);

  testbench.write_verilog(exported_testbench, params);

  z3::model s_model = solver->get_model();

  // generating an assignment for a verilog model using SMT, then writing the inputs and initial state values into a testbench, comparing the output values for the timesteps after the initial state between SMT model and verilog simulation trace

  spdlog::info("Simulating Exported Design + Testbench");

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;
  auto data = ducode::simulate_design(exported_verilog, exported_testbench);
  spdlog::info("End of Simulation");
  vcd_data_vector.emplace_back(data);

  std::vector<const VCDScope *> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto &vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  CHECK(smt_to_simulation_validation(instance, ev_edges, solver, signal_name_to_edge_id, unroll_factor));

  // call simulation
}

TEST_CASE("smt_to_simulation_validation_usb2uart", "[smt-sim_validation][advanced][.]") {
  nlohmann::json params;
  params["ift"] = false;
  bool smt_debug = false;

  spdlog::info("1. Parse Design JSON file");
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "usb2uart" / "usb2uart.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  params["clock"] = "clk";
  const uint32_t tag_size = 512;
  params["tag_size"] = tag_size;
  // read in json netlist
  design.set_clk_signal_name("clk");
  design.set_top_module_name("core");

  //instance.write_verilog("test.v");
  //instance.write_graphviz("graph_unrolling_usb2uart.dot");

  // call design verilog export
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  // setup exported verilog file paths
  auto exported_verilog = temp_dir / "smt_validation_design.v";
  auto exported_testbench = temp_dir / "smt_validation_testbench.v";

  // export ift enhanced verilog design file
  design.write_verilog(exported_verilog, params);

  spdlog::info("2. Setup SMT solver");
  // z3 test starts here
  auto ctx = std::make_shared<z3::context>();
  auto solver = std::make_shared<z3::solver>(*ctx);
  auto ev_edges = std::make_shared<z3::expr_vector>(*ctx);
  uint32_t unroll_factor = 4;
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 0);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 1);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 2);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 3);

  REQUIRE(solver->check() == z3::sat);

  spdlog::info("3. Setup Verilog Simulation");
  std::vector<std::string> hierarchy = {design.get_top_module().get_name()};
  auto input_names = design.get_top_module().get_input_names();
  std::vector<ducode::SignalIdentifier> input_ids;
  input_ids.reserve(input_names.size());
  for (auto &iname: input_names) {
    if (iname == design.get_clk_signal_name()) {
      continue;
    }
    input_ids.push_back({iname, hierarchy});
  }

  auto output_names = design.get_top_module().get_output_names();
  std::vector<ducode::SignalIdentifier> output_ids;
  output_ids.reserve(output_names.size());
  for (auto &oname: output_names) {
    output_ids.push_back({oname, hierarchy});
  }
  auto testbench_value_map = std::make_shared<ducode::SMTSignalsDataManager>(ctx, solver, signal_name_to_edge_id, unroll_factor, ev_edges, input_ids, output_ids, design.get_top_module_name());
  ducode::TestbenchSMT testbench(design, testbench_value_map, instance);

  testbench.write_verilog(exported_testbench, params);

  z3::model s_model = solver->get_model();

  // generating an assignment for a verilog model using SMT, then writing the inputs and initial state values into a testbench, comparing the output values for the timesteps after the initial state between SMT model and verilog simulation trace

  spdlog::info("Simulating Exported Design + Testbench");

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;
  auto data = ducode::simulate_design(exported_verilog, exported_testbench);
  spdlog::info("End of Simulation");
  vcd_data_vector.emplace_back(data);

  std::vector<const VCDScope *> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto &vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  CHECK(smt_to_simulation_validation(instance, ev_edges, solver, signal_name_to_edge_id, unroll_factor));

  // call simulation
}

TEST_CASE("smt_to_simulation_validation_yadmc", "[smt-sim_validation][advanced][.]") {
  nlohmann::json params;
  params["ift"] = false;
  bool smt_debug = false;

  spdlog::info("1. Parse Design JSON file");
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "yadmc" / "yadmc.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  params["clock"] = "clk";
  const uint32_t tag_size = 512;
  params["tag_size"] = tag_size;
  // read in json netlist
  design.set_clk_signal_name("clk");
  design.set_top_module_name("yadmc");

  //instance.write_verilog("test.v");
  //instance.write_graphviz("graph_unrolling_yadmc.dot");

  // call design verilog export
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  // setup exported verilog file paths
  auto exported_verilog = temp_dir / "smt_validation_design.v";
  auto exported_testbench = temp_dir / "smt_validation_testbench.v";

  // export ift enhanced verilog design file
  design.write_verilog(exported_verilog, params);

  spdlog::info("2. Setup SMT solver");
  // z3 test starts here
  auto ctx = std::make_shared<z3::context>();
  auto solver = std::make_shared<z3::solver>(*ctx);
  auto ev_edges = std::make_shared<z3::expr_vector>(*ctx);
  uint32_t unroll_factor = 4;
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 0);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 1);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 2);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 3);

  spdlog::info("Checking SMT Solver Satisfiability");
  REQUIRE(solver->check() == z3::sat);

  spdlog::info("3. Setup Verilog Simulation");
  std::vector<std::string> hierarchy = {design.get_top_module().get_name()};
  auto input_names = design.get_top_module().get_input_names();
  std::vector<ducode::SignalIdentifier> input_ids;
  input_ids.reserve(input_names.size());
  for (auto &iname: input_names) {
    if (iname == design.get_clk_signal_name()) {
      continue;
    }
    input_ids.push_back({iname, hierarchy});
  }

  auto output_names = design.get_top_module().get_output_names();
  std::vector<ducode::SignalIdentifier> output_ids;
  output_ids.reserve(output_names.size());
  for (auto &oname: output_names) {
    output_ids.push_back({oname, hierarchy});
  }
  auto testbench_value_map = std::make_shared<ducode::SMTSignalsDataManager>(ctx, solver, signal_name_to_edge_id, unroll_factor, ev_edges, input_ids, output_ids, design.get_top_module_name());
  ducode::TestbenchSMT testbench(design, testbench_value_map, instance);

  testbench.write_verilog(exported_testbench, params);

  z3::model s_model = solver->get_model();

  // generating an assignment for a verilog model using SMT, then writing the inputs and initial state values into a testbench, comparing the output values for the timesteps after the initial state between SMT model and verilog simulation trace

  spdlog::info("Simulating Exported Design + Testbench");

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;
  auto data = ducode::simulate_design(exported_verilog, exported_testbench);
  spdlog::info("End of Simulation");
  vcd_data_vector.emplace_back(data);

  std::vector<const VCDScope *> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto &vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  CHECK(smt_to_simulation_validation(instance, ev_edges, solver, signal_name_to_edge_id, unroll_factor));

  // call simulation
}

TEST_CASE("smt_to_simulation_validation_SPI_Slave", "[smt-sim_validation][advanced][.]") {
  nlohmann::json params;
  params["ift"] = false;
  bool smt_debug = false;

  spdlog::info("1. Parse Design JSON file");
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "SPI_Slave" / "SPI_Slave.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  //instance.write_graphviz("instancegraph_SPI_Slave.dot");

  params["clock"] = "slaveSCLK";
  const uint32_t tag_size = 512;
  params["tag_size"] = tag_size;
  // read in json netlist
  design.set_clk_signal_name("slaveSCLK");
  design.set_top_module_name("SPI_Slave");

  //instance.write_verilog("test.v");
  //instance.write_graphviz("graph_unrolling_SPI_Slave.dot");

  // call design verilog export
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  // setup exported verilog file paths
  auto exported_verilog = temp_dir / "smt_validation_design.v";
  auto exported_testbench = temp_dir / "smt_validation_testbench.v";

  // export ift enhanced verilog design file
  design.write_verilog(exported_verilog, params);

  spdlog::info("2. Setup SMT solver");
  // z3 test starts here
  auto ctx = std::make_shared<z3::context>();
  auto solver = std::make_shared<z3::solver>(*ctx);
  auto ev_edges = std::make_shared<z3::expr_vector>(*ctx);
  uint32_t unroll_factor = 4;
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 0);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 1);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 2);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 3);

  spdlog::info("Checking SMT Solver Satisfiability");
  REQUIRE(solver->check() == z3::sat);

  spdlog::info("3. Setup Verilog Simulation");
  std::vector<std::string> hierarchy = {design.get_top_module().get_name()};
  auto input_names = design.get_top_module().get_input_names();
  std::vector<ducode::SignalIdentifier> input_ids;
  input_ids.reserve(input_names.size());
  for (auto &iname: input_names) {
    if (iname == design.get_clk_signal_name()) {
      continue;
    }
    input_ids.push_back({iname, hierarchy});
  }

  auto output_names = design.get_top_module().get_output_names();
  std::vector<ducode::SignalIdentifier> output_ids;
  output_ids.reserve(output_names.size());
  for (auto &oname: output_names) {
    output_ids.push_back({oname, hierarchy});
  }
  auto testbench_value_map = std::make_shared<ducode::SMTSignalsDataManager>(ctx, solver, signal_name_to_edge_id, unroll_factor, ev_edges, input_ids, output_ids, design.get_top_module_name());
  ducode::TestbenchSMT testbench(design, testbench_value_map, instance);

  testbench.write_verilog(exported_testbench, params);

  z3::model s_model = solver->get_model();

  // generating an assignment for a verilog model using SMT, then writing the inputs and initial state values into a testbench, comparing the output values for the timesteps after the initial state between SMT model and verilog simulation trace

  spdlog::info("Simulating Exported Design + Testbench");

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;
  auto data = ducode::simulate_design(exported_verilog, exported_testbench);
  spdlog::info("End of Simulation");
  vcd_data_vector.emplace_back(data);

  std::vector<const VCDScope *> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto &vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  CHECK(smt_to_simulation_validation(instance, ev_edges, solver, signal_name_to_edge_id, unroll_factor));

  // call simulation
}

TEST_CASE("smt_to_simulation_validation_eth_wishbone", "[smt-sim_validation][advanced][.]") {
  nlohmann::json params;
  params["ift"] = false;
  bool smt_debug = false;

  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "eth_wishbone" / "eth_wishbone.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  std::string clock = "WB_CLK_I";
  params["clock"] = clock;
  const uint32_t tag_size = 512;
  params["tag_size"] = tag_size;
  // read in json netlist
  design.set_clk_signal_name(clock);
  design.set_top_module_name("eth_wishbone");

  //instance.write_verilog("test.v");
  //instance.write_graphviz("graph_unrolling_eth_wishbone.dot");

  // call design verilog export
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  // setup exported verilog file paths
  auto exported_verilog = temp_dir / "smt_validation_design.v";
  auto exported_testbench = temp_dir / "smt_validation_testbench.v";

  // export ift enhanced verilog design file
  design.write_verilog(exported_verilog, params);

  // z3 test starts here
  auto ctx = std::make_shared<z3::context>();
  auto solver = std::make_shared<z3::solver>(*ctx);
  auto ev_edges = std::make_shared<z3::expr_vector>(*ctx);
  uint32_t unroll_factor = 4;
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 0);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 1);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 2);
  instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, 3);

  REQUIRE(solver->check() == z3::sat);

  std::vector<std::string> hierarchy = {design.get_top_module().get_name()};
  auto input_names = design.get_top_module().get_input_names();
  std::vector<ducode::SignalIdentifier> input_ids;
  input_ids.reserve(input_names.size());
  for (auto &iname: input_names) {
    if (iname == design.get_clk_signal_name()) {
      continue;
    }
    input_ids.push_back({iname, hierarchy});
  }

  auto output_names = design.get_top_module().get_output_names();
  std::vector<ducode::SignalIdentifier> output_ids;
  output_ids.reserve(output_names.size());
  for (auto &oname: output_names) {
    output_ids.push_back({oname, hierarchy});
  }
  auto testbench_value_map = std::make_shared<ducode::SMTSignalsDataManager>(ctx, solver, signal_name_to_edge_id, unroll_factor, ev_edges, input_ids, output_ids, design.get_top_module_name());
  ducode::TestbenchSMT testbench(design, testbench_value_map, instance);

  testbench.write_verilog(exported_testbench, params);

  z3::model s_model = solver->get_model();

  // generating an assignment for a verilog model using SMT, then writing the inputs and initial state values into a testbench, comparing the output values for the timesteps after the initial state between SMT model and verilog simulation trace

  spdlog::info("Simulating Exported Design + Testbench");

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;
  auto data = ducode::simulate_design(exported_verilog, exported_testbench);
  spdlog::info("End of Simulation");
  vcd_data_vector.emplace_back(data);

  std::vector<const VCDScope *> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto &vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  CHECK(smt_to_simulation_validation(instance, ev_edges, solver, signal_name_to_edge_id, unroll_factor));

  // call simulation
}

TEST_CASE("smt_to_simulation_validation_randomized", "[smt-sim_validation][advanced][randomized]") {
  nlohmann::json params;
  params["ift"] = false;

  uint32_t unroll_factor = 4;
  const uint32_t constUnrollEight = 8;
  const uint32_t constCombUnroll = 1;
  const uint32_t constUnrollTwo = 2;

  const uint32_t defaultNumstimuli = 16;
  uint32_t numstimuli = defaultNumstimuli;

  boost::filesystem::path json_file;
  params["clock"] = "";
  auto tfDir = boost::filesystem::path{TESTFILES_DIR};

  SECTION("basic_gates") {
    params["top"] = "top";

    json_file = tfDir / "basic_gates" / "gates_verilog" / GENERATE(
                                                              // pow not supported on bv:  "pow.json",
                                                              "add.json", "logic_and.json", "and.json", "logic_or.json", "shl.json", "div.json", "lt.json", "shr.json", "eq.json", "sshl.json", "eqx.json", "mul.json", "ge.json", "ne.json", "sub.json", "gt.json", "nex.json", "xnor.json", "le.json", "or.json", "xor.json",
                                                              // We may have a problem with signedness:
                                                              "mod.json", "sshr.json");
  }


  SECTION("dff_design") {
    json_file = boost::filesystem::path{TESTFILES_DIR} / "dff_design" / "dff_design.json";
    params["clock"] = "clk";
    params["top"] = "dff_design";
  }

  SECTION("mealy_moore") {
    json_file = boost::filesystem::path{TESTFILES_DIR} / "mealy_moore" / "mealy_moore.json";
    params["clock"] = "clk";
    params["top"] = "mealy_moore";
  }

  SECTION("pass_through") {
    json_file = boost::filesystem::path{TESTFILES_DIR} / "pass_through" / "pass_through.json";
    params["clock"] = "clk";
    params["top"] = "pass_through";
  }

  /* The dff_x design is bound to fail - we do not handle x-values in SMT
  SECTION("dff_x")
  {
    json_file = boost::filesystem::path{TESTFILES_DIR} / "dff_design_x" / "dff_design.json";
    params["clock"] = "clk";
    params["top"] = "dff_design";
  }
  */

  SECTION("qifa_exampleCombinational") {
    json_file = boost::filesystem::path{TESTFILES_DIR} / "qifa_example" / "qifa_exampleCombinational.json";
    //params["clock"] = "clk";
    params["top"] = "qifa_exampleSimple";
  }

  SECTION("qifa_exampleSimple") {
    json_file = boost::filesystem::path{TESTFILES_DIR} / "qifa_example" / "qifa_exampleSimple.json";
    params["clock"] = "clk";
    params["top"] = "qifa_exampleSimple";
  }

  spdlog::info(std::string("Using JSON file ") + json_file.string());

  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  const uint32_t tag_size = 512;
  params["tag_size"] = tag_size;
  // read in json netlist
  if (params["clock"] != std::string("")) {
    design.set_clk_signal_name(params["clock"]);
  }
  design.set_top_module_name(params["top"]);

  // call design verilog export
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  // setup exported verilog file paths
  auto exported_verilog = temp_dir / "smt_validation_design.v";

  // export ift enhanced verilog design file
  design.write_verilog(exported_verilog, params);
  spdlog::info(std::string("Exported verilog file to ") + exported_verilog.string());

  // z3 test starts here
  auto ctx = std::make_shared<z3::context>();
  auto solver = std::make_shared<z3::solver>(*ctx);
  auto ev_edges = std::make_shared<z3::expr_vector>(*ctx);
  const uint32_t design_instance_id = 1;

  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)
  std::vector<z3::expr> stateInit_expr;
  std::vector<std::vector<z3::expr>> input_expr;
  std::vector<std::vector<z3::expr>> output_expr;
  std::vector<std::string> hierarchy = {design.get_top_module().get_name()};
  // Unroll and collect z3-expressions for state/in/out
  for (uint32_t time = 0; time < unroll_factor; time++) {
    instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, design_instance_id, time);
    std::vector<z3::expr> input_expr_time;
    std::vector<z3::expr> output_expr_time;
    for (auto &name_in_design: design.get_top_module().get_input_names()) {
      auto smtname = ducode::create_smt_signal_name(design_instance_id, hierarchy, name_in_design, time);
      spdlog::info("Input-variable: {}", smtname);
      auto elem = signal_name_to_edge_id.find(smtname);
      if (elem != signal_name_to_edge_id.end()) {
        input_expr_time.push_back((*ev_edges)[elem->second]);
        spdlog::info("      expr: {}", input_expr_time.back().to_string());
      }
    }
    input_expr.push_back(input_expr_time);
    for (auto &name_in_design: design.get_top_module().get_output_names()) {
      auto smtname = ducode::create_smt_signal_name(design_instance_id, hierarchy, name_in_design, time);
      spdlog::info("Output-variable: {}", smtname);
      auto elem = signal_name_to_edge_id.find(smtname);
      if (elem != signal_name_to_edge_id.end()) {
        output_expr_time.push_back((*ev_edges)[elem->second]);
        spdlog::info("      expr: {}", output_expr_time.back().to_string());
      }
    }
    output_expr.push_back(output_expr_time);
    if (time == 0) {
      for (auto &ff_in_design: instance.get_all_flipflop_ids()) {
        auto smtname = ducode::create_smt_signal_name(design_instance_id, ff_in_design.hierarchy, ff_in_design.name, time);
        spdlog::info("Output-variable: {}", smtname);
        auto elem = signal_name_to_edge_id.find(smtname);
        if (elem != signal_name_to_edge_id.end()) {
          stateInit_expr.push_back((*ev_edges)[elem->second]);
          spdlog::info("      expr: {}", stateInit_expr.back().to_string());
        }
      }
    }
  }

  //std::string smt2_filename = "smtInstanceUnrolled.smt2";
  //ducode::write_to_smt2(smt2_filename, *solver);
  REQUIRE(solver->check() == z3::sat);

  auto input_names = design.get_top_module().get_input_names();
  std::vector<ducode::SignalIdentifier> input_ids;
  input_ids.reserve(input_names.size());
  for (auto &iname: input_names) {
    if (iname == design.get_clk_signal_name()) {
      continue;
    }
    input_ids.push_back({iname, hierarchy});
  }
  auto output_names = design.get_top_module().get_output_names();
  std::vector<ducode::SignalIdentifier> output_ids;
  output_ids.reserve(output_names.size());
  for (auto &oname: output_names) {
    output_ids.push_back({oname, hierarchy});
  }

  for (uint32_t cntstimuli = 0; cntstimuli < numstimuli; cntstimuli++) {
    solver->push();
    spdlog::info("Random assignment number {}", cntstimuli);
    // Randomly assign values in SMT instance to inputs and initial state
    for (const auto &expr: stateInit_expr) {
      ducode::smt_random_assignment(*ctx, *solver, expr);
      //GF for debugging: ducode::smt_fixed_assignment(*ctx, *solver, expr, 3);
    }
    for (const auto &vec_expr: input_expr) {
      for (const auto &expr: vec_expr) {
        ducode::smt_random_assignment(*ctx, *solver, expr);
        //GF for debugging: ducode::smt_fixed_assignment(*ctx, *solver, expr, 1);
      }
    }
    REQUIRE(solver->check() == z3::sat);

    auto exported_testbench = temp_dir / (std::string("smt_testbench_") + std::to_string(cntstimuli) + ".v");
    //boost::filesystem::path exported_testbench = ( std::string("smt_testbench_") + std::to_string(cntstimuli)+".v" );
    spdlog::info("Simulating Exported Design + Testbench " + exported_testbench.string());
    auto testbench_value_map = std::make_shared<ducode::SMTSignalsDataManager>(ctx, solver, signal_name_to_edge_id, unroll_factor, ev_edges, input_ids, output_ids, design.get_top_module_name());
    ducode::TestbenchSMT testbench(design, testbench_value_map, instance);
    testbench.write_verilog(exported_testbench, params);
    std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;
    auto data = ducode::simulate_design(exported_verilog, exported_testbench);
    spdlog::info("End of Simulation");
    vcd_data_vector.emplace_back(data);

    std::vector<const VCDScope *> vcd_root_scopes;
    vcd_root_scopes.reserve(vcd_data_vector.size());
    for (auto &vcd_file: vcd_data_vector) {
      vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
    }
    ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
    instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

    // Check whether simulation and solver return the same result
    spdlog::info("Checking whether simulation yields identical results");
    bool identical = smt_to_simulation_validation(instance, ev_edges, solver, signal_name_to_edge_id, unroll_factor);
    if (!identical) {
      CHECK(validation_breadth_first_search(instance, *ev_edges, *solver, *ctx, input_names, signal_name_to_edge_id, unroll_factor));
    }
    CHECK(identical);

    // Check whether this is the only output for these inputs
    spdlog::info("Checking whether assignment yields deterministc output");
    CHECK(ducode::smt_to_simulation_unique_assignment(*ctx, *solver, stateInit_expr, input_expr, output_expr));
    solver->pop();
  }

  spdlog::info(std::string("Test ended for ") + json_file.string());
}
