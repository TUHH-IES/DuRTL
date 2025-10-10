#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/instantiation_graph.hpp>
#include <ducode/instantiation_graph_traits.hpp>
#include <ducode/signals_data_manager.hpp>
#include <ducode/tag_generator.hpp>
#include <ducode/testbench.hpp>
#include <ducode/utility/VCD_utility.hpp>
#include <ducode/utility/iverilog_wrapper.hpp>
#include <ducode/utility/simulation.hpp>
#include <ducode/utility/smt.hpp>// NOLINT(misc-include-cleaner)
#include <ducode/utility/types.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/graph/detail/adjacency_list.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <gsl/narrow>
#include <nlohmann/json_fwd.hpp>
#include <spdlog/spdlog.h>
#include <vcd-parser/VCDFile.hpp>
#include <vcd-parser/VCDTypes.hpp>
#include <z3++.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

TEST_CASE("ifa_smt_vcd", "[ifa_smt2]") {
  auto verilog_file = boost::filesystem::path(TESTFILES_DIR) / "and" / "and.v";
  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / "and" / "and_tb.v";
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "and" / "and.json";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);
  // setup exported verilog file paths
  auto exported_verilog = temp_dir / "ift_design.v";
  auto exported_testbench = temp_dir / "ift_testbench.v";

  // read in json netlist
  auto design = ducode::Design::parse_json(json_file);

  // simulate design with original verilog + testbench
  auto vcd_data = ducode::simulate_design(verilog_file, testbench_file);

  // export ift enhanced verilog design file
  nlohmann::json params;
  params["ift"] = true;
  const uint32_t tag_size = 512;
  params["tag_size"] = tag_size;
  design.write_verilog(exported_verilog, params);
  auto stepsize = gsl::narrow<uint32_t>(ducode::get_stepsize(vcd_data, "clk"));
  auto top_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });
  assert(top_module != design.get_modules().end());

  auto timesteps_per_simulation_run = tag_size / top_module->get_input_names().size();
  timesteps_per_simulation_run = std::max<size_t>(timesteps_per_simulation_run, 1);
  auto number_simulation_runs = ducode::get_number_of_simulation_runs(stepsize, timesteps_per_simulation_run, vcd_data);
  params["timesteps_per_simulation_run"] = timesteps_per_simulation_run;

  ducode::DesignInstance instance = ducode::DesignInstance::create_instance(design);

  const VCDScope* root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));
  testbench.set_tag_generator(std::make_unique<ducode::FullResolutionTagGenerator>(top_module->get_input_ports(), vcd_data));

  std::vector<VCDTime> tags_times;
  for (auto time = 0u; time <= gsl::narrow<std::size_t>(vcd_data->get_timestamps().back()); time += stepsize) {
    tags_times.emplace_back(time);
  }
  std::vector<boost::filesystem::path> exported_tags_files;
  for (uint32_t i = 0; i < number_simulation_runs; ++i) {
    boost::filesystem::path tag_file = temp_dir / ("vcd_input_" + std::to_string(i) + ".txt");
    exported_tags_files.push_back(tag_file);
  }

  testbench.write_verilog(exported_testbench, params);
  std::vector<ExportedTagsFile> exported_tags;

  /// here add loop which runs the simulation again for all created tag_map files.
  for (uint32_t i = 0; i < number_simulation_runs; ++i) {
    // For the ith batch of timestamps
    uint32_t items_per_batch = timesteps_per_simulation_run;
    uint32_t start_idx = i * items_per_batch;
    uint32_t end_idx = std::min(((i + 1) * items_per_batch), gsl::narrow<uint32_t>(tags_times.size()));

    std::vector<VCDTime> batch(tags_times.begin() + start_idx,
                               tags_times.begin() + end_idx);

    exported_tags.emplace_back(batch, exported_tags_files[i]);
  }
  testbench.write_tags(exported_tags, params);

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;
  for (const auto& exported_tags: exported_tags_files) {
    /// IFT simulation
    spdlog::info("Simulating Exported Design + Testbench");
    auto data = ducode::simulate_design(exported_verilog, exported_testbench, exported_tags);
    spdlog::info("End of Simulation");
    vcd_data_vector.emplace_back(data);
  }
  std::vector<const VCDScope*> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto& vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  std::shared_ptr<ducode::SignalsDataManager> test_value_map_vcd = std::make_shared<ducode::VCDSignalsDataManager>(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(test_value_map_vcd);

  CHECK(boost::num_vertices(instance.m_graph) == 4);
  CHECK(boost::num_edges(instance.m_graph) == 3);

  // z3 test starts here
  auto ctx = std::make_shared<z3::context>();
  auto solver = std::make_shared<z3::solver>(*ctx);
  auto ev_edges = std::make_shared<z3::expr_vector>(*ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  for (uint32_t time = 0; time < 4; ++time) {
    instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 1, time);
    instance.export_smt2(*ctx, *solver, *ev_edges, signal_name_to_edge_id, 2, time);
  }
  ducode::SMTSignalsDataManager tracker(ctx, solver, signal_name_to_edge_id, 1, ev_edges, design.get_input_signal_ids(), design.get_output_signal_ids(), design.get_top_module_name());
  std::shared_ptr<ducode::SignalsDataManager> test_value_map_smt = std::make_shared<ducode::SMTSignalsDataManager>(tracker);

  auto test_signal_tracker = GENERATE_COPY(test_value_map_smt, test_value_map_vcd);
  instance.add_signal_tag_map(test_signal_tracker);

  ducode::DesignInstance::write_smt2(*solver, "smt_and.smt2");

  ducode::SignalIdentifier input_signal = {.name = "b", .hierarchy = {"anda"}};
  double input_time = 1;
  ducode::SignalIdentifier target_signal = {.name = "c", .hierarchy = {"anda"}};
  double target_time = 1;

  CHECK(instance.if_signal_to_signal(input_signal, input_time, target_signal, target_time));

  input_signal = {.name = "a", .hierarchy = {"anda"}};
  input_time = 2;
  target_signal = {.name = "c", .hierarchy = {"anda"}};
  target_time = 2;

  CHECK(instance.if_signal_to_signal(input_signal, input_time, target_signal, target_time));

  input_signal = {.name = "a", .hierarchy = {"anda"}};
  input_time = 3;
  target_signal = {.name = "c", .hierarchy = {"anda"}};
  target_time = 3;

  CHECK(instance.if_signal_to_signal(input_signal, input_time, target_signal, target_time));

  input_signal = {.name = "b", .hierarchy = {"anda"}};
  input_time = 3;
  target_signal = {.name = "c", .hierarchy = {"anda"}};
  target_time = 3;

  CHECK(instance.if_signal_to_signal(input_signal, input_time, target_signal, target_time));
}

TEST_CASE("vcd_ifa_hierarchy", "[ifa_vcd]") {
  auto verilog_file = boost::filesystem::path(TESTFILES_DIR) / "hierarchy" / "hierarchy.v";
  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / "hierarchy" / "hierarchy_tb.v";
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "hierarchy" / "hierarchy.json";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);
  // setup exported verilog file paths
  auto exported_verilog = temp_dir / "ift_design.v";
  auto exported_testbench = temp_dir / "ift_testbench.v";

  // read in json netlist
  auto design = ducode::Design::parse_json(json_file);

  // simulate design with original verilog + testbench
  auto vcd_data = ducode::simulate_design(verilog_file, testbench_file);

  // export ift enhanced verilog design file
  nlohmann::json params;
  params["ift"] = true;
  const uint32_t tag_size = 512;
  params["tag_size"] = tag_size;
  design.write_verilog(exported_verilog, params);

  auto stepsize = gsl::narrow<uint32_t>(ducode::get_stepsize(vcd_data, "clk"));
  auto top_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });
  assert(top_module != design.get_modules().end());

  auto timesteps_per_simulation_run = tag_size / top_module->get_input_names().size();
  timesteps_per_simulation_run = std::max<size_t>(timesteps_per_simulation_run, 1);
  auto number_simulation_runs = ducode::get_number_of_simulation_runs(stepsize, timesteps_per_simulation_run, vcd_data);

  params["timesteps_per_simulation_run"] = timesteps_per_simulation_run;

  ducode::DesignInstance instance = ducode::DesignInstance::create_instance(design);

  const VCDScope* root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));
  testbench.set_tag_generator(std::make_unique<ducode::FullResolutionTagGenerator>(top_module->get_input_ports(), vcd_data));

  std::vector<VCDTime> tags_times;
  for (auto time = 0u; time <= gsl::narrow<std::size_t>(vcd_data->get_timestamps().back()); time += stepsize) {
    tags_times.emplace_back(time);
  }
  std::vector<boost::filesystem::path> exported_tags_files;
  for (uint32_t i = 0; i < number_simulation_runs; ++i) {
    boost::filesystem::path tag_file = temp_dir / ("vcd_input_" + std::to_string(i) + ".txt");
    exported_tags_files.push_back(tag_file);
  }

  testbench.write_verilog(exported_testbench, params);
  std::vector<ExportedTagsFile> exported_tags;

  /// here add loop which runs the simulation again for all created tag_map files.
  for (uint32_t i = 0; i < number_simulation_runs; ++i) {
    // For the ith batch of timestamps
    uint32_t items_per_batch = timesteps_per_simulation_run;
    uint32_t start_idx = i * items_per_batch;
    uint32_t end_idx = std::min(((i + 1) * items_per_batch), gsl::narrow<uint32_t>(tags_times.size()));

    std::vector<VCDTime> batch(tags_times.begin() + start_idx,
                               tags_times.begin() + end_idx);

    exported_tags.emplace_back(batch, exported_tags_files[i]);
  }
  testbench.write_tags(exported_tags, params);

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;
  for (const auto& exported_tags: exported_tags_files) {
    /// IFT simulation
    spdlog::info("Simulating Exported Design + Testbench");
    auto data = ducode::simulate_design(exported_verilog, exported_testbench, exported_tags);
    spdlog::info("End of Simulation");
    vcd_data_vector.emplace_back(data);
  }
  std::vector<const VCDScope*> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto& vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map_vcd(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_shared<ducode::VCDSignalsDataManager>(test_value_map_vcd));

  ducode::SignalIdentifier input_signal = {.name = "b", .hierarchy = {"hierarchy"}};
  double input_time = 1;
  ducode::SignalIdentifier target_signal = {.name = "c", .hierarchy = {"hierarchy"}};
  double target_time = 1;

  CHECK(instance.if_signal_to_signal(input_signal, input_time, target_signal, target_time));
}
