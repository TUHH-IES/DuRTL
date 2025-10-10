#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/instantiation_graph.hpp>
#include <ducode/utility/smt.hpp>// NOLINT(misc-include-cleaner)

#include <boost/graph/detail/adjacency_list.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <z3++.h>

#include <cstdint>
#include <string>
#include <unordered_map>

TEST_CASE("smt_dff_export", "[smt2_unroll]") {
  // "aldff", "adff", "adffe", "aldffe" cells missing
  const auto* ff_type = GENERATE("dff", "dffe", "dlatch", "dffsr", "dffsre", "adlatch", "dlatchsr");

  spdlog::info(ff_type);
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "flipFlops" / ff_type / fmt::format("{}.json", ff_type);
  // auto json_file = boost::filesystem::path{TESTFILES_DIR} / "dff" / "dff.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  //instance.write_verilog(fmt::format("{}_test.v", ff_type));
  //instance.write_graphviz(fmt::format("{}_graph.dot", ff_type));

  // z3 test starts here
  z3::context ctx;
  z3::solver solver(ctx);
  z3::expr_vector ev_edges(ctx);
  uint32_t unroll_factor = 2;
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, 0);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, 1);

  //ducode::DesignInstance::write_smt2(solver, "dff.smt2");

  spdlog::info("{}", ev_edges);

  solver.push();
  solver.add(ev_edges[0] == 0);// clk_0
  solver.add(ev_edges[1] == 0);// d_0
  solver.add(ev_edges[3] == 1);// clk_1
  solver.add(ev_edges[4] == 1);// d_1

  CHECK(solver.check() == z3::sat);
  spdlog::debug("{}", solver.to_smt2());

  spdlog::debug("{}", solver.get_model());
  solver.pop();
}

TEST_CASE("smt_sdff", "[smt2_unroll]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "flipFlops" / "sdff" / "sdff.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  //instance.write_verilog("test.v");
  //instance.write_graphviz("graph.dot");

  CHECK(boost::num_vertices(instance.m_graph) == 5);
  CHECK(boost::num_edges(instance.m_graph) == 4);

  // z3 test starts here
  z3::context ctx;
  z3::solver solver(ctx);
  z3::expr_vector ev_edges(ctx);
  uint32_t unroll_factor = 2;
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, 0);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, 1);

  spdlog::info("{}", ev_edges);
  const int input_d1 = 5;
  solver.add(ev_edges[0] == 0);       // clk_0
  solver.add(ev_edges[1] == 0);       // d_0
  solver.add(ev_edges[4] == 1);       // clk_1
  solver.add(ev_edges[input_d1] == 1);// d_1

  CHECK(solver.check() == z3::sat);
  spdlog::debug("{}", solver.to_smt2());
  spdlog::debug("{}", solver.get_model());
}

TEST_CASE("smt_sdffe", "[smt2_unroll]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "flipFlops" / "sdffe" / "sdffe.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  //instance.write_verilog("test.v");
  //instance.write_graphviz("graph.dot");

  CHECK(boost::num_vertices(instance.m_graph) == 6);
  CHECK(boost::num_edges(instance.m_graph) == 5);

  // z3 test starts here
  z3::context ctx;
  z3::solver solver(ctx);
  z3::expr_vector ev_edges(ctx);
  uint32_t unroll_factor = 2;
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, 0);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, 1);

  spdlog::info("{}", ev_edges);
  const int input_d1 = 5;
  solver.add(ev_edges[0] == 0);       // clk_0
  solver.add(ev_edges[1] == 0);       // d_0
  solver.add(ev_edges[4] == 1);       // clk_1
  solver.add(ev_edges[input_d1] == 1);// d_1

  CHECK(solver.check() == z3::sat);
  spdlog::debug("{}", solver.to_smt2());
  spdlog::debug("{}", solver.get_model());
}

TEST_CASE("smt_dff_2x", "[smt2_unroll]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "dff_2x" / "dff_2x.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  //instance.write_verilog("test.v");
  //instance.write_graphviz("graph_unrolling.dot");

  CHECK(boost::num_vertices(instance.m_graph) == 5);
  CHECK(boost::num_edges(instance.m_graph) == 5);

  // z3 test starts here
  z3::context ctx;
  z3::solver solver(ctx);
  z3::expr_vector ev_edges(ctx);
  uint32_t unroll_factor = 2;
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, 0);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, 1);

  spdlog::info("{}", ev_edges);

  solver.add(ev_edges[0] == 1);// clk_0
  solver.add(ev_edges[1] == 0);// d_0
  solver.add(ev_edges[4] == 1);// clk_1
  const int input_1 = 5;
  solver.add(ev_edges[input_1] == 1);// d_1

  CHECK(solver.check() == z3::sat);
  spdlog::debug("{}", solver.to_smt2());
  spdlog::debug("{}", solver.get_model());
}

TEST_CASE("smt_dff_design", "[smt2_unroll]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "dff_design" / "dff_design.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  //instance.write_verilog("test.v");
  //instance.write_graphviz("graph_unrolling_dff_design.dot");

  CHECK(boost::num_vertices(instance.m_graph) == 12);
  CHECK(boost::num_edges(instance.m_graph) == 14);

  // z3 test starts here
  z3::context ctx;
  z3::solver solver(ctx);
  z3::expr_vector ev_edges(ctx);
  uint32_t unroll_factor = 2;
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, 0);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, 1);

  spdlog::info("{}", ev_edges);

  solver.add(ev_edges[0] == 1);// clk_0
  solver.add(ev_edges[1] == 0);// d_0

  CHECK(solver.check() == z3::sat);

  spdlog::debug("{}", solver.to_smt2());
  spdlog::debug("{}", solver.get_model());
  z3::model m = solver.get_model();
  spdlog::info("{} === {}", ev_edges[0].to_string(), m.eval(ev_edges[0]).as_int64());
  spdlog::info("{}\n stop \n", solver.get_model());
}

TEST_CASE("smt_design_export_test_nonworking", "[smt2_unroll][.]") {
  // "aes_128", "i2c_controller", "mips_16_core_top"
  const auto* design_name = GENERATE("i2c_controller", "mips_16_core_top", "fft16", "y86");

  spdlog::info(design_name);
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / design_name / fmt::format("{}.json", design_name);
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  //instance.write_verilog("test.v");
  //instance.write_graphviz("graph_unrolling_aes128_design.dot");

  // z3 test starts here
  z3::context ctx;
  z3::solver solver(ctx);
  z3::expr_vector ev_edges(ctx);
  uint32_t unroll_factor = 2;
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, 0);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, 1);


  spdlog::info("{}", solver.to_smt2());

  CHECK(solver.check() == z3::sat);

  if (solver.check() == z3::sat) {
    spdlog::debug("{}", solver.to_smt2());
    spdlog::debug("{}", solver.get_model());
  }
}
