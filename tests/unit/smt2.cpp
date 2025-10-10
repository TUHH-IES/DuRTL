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
//#include <z3_api.h>

TEST_CASE("smt_and", "[smt2]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "basic_gates" / "binary_gates" / "and" / "and.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  CHECK(boost::num_vertices(instance.m_graph) == 4);
  CHECK(boost::num_edges(instance.m_graph) == 3);

  // z3 test starts here
  z3::context ctx;
  z3::solver solver(ctx);
  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id);

  ducode::DesignInstance::write_smt2(solver, "smt_and.smt2");

  for (uint32_t i = 0; i < 2; i++) {
    for (uint32_t j = 0; j < 2; j++) {
      solver.push();
      solver.add(ev_edges[0] == static_cast<int>(i));
      solver.add(ev_edges[1] == static_cast<int>(j));

      CHECK(solver.check() == z3::sat);
      spdlog::debug("{}", solver.to_smt2());
      z3::model solver_model = solver.get_model();
      spdlog::debug("{} == {}", solver.get_model().eval(ev_edges[2]).as_int64(), (i & j));
      CHECK(solver.get_model().eval(ev_edges[2]).as_int64() == (i & j));

      solver.pop();
    }
  }
}

TEST_CASE("smt_qifa", "[smt2]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "qifa_example" / "qifa_exampleCombinational.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  CHECK(boost::num_vertices(instance.m_graph) == 7);
  CHECK(boost::num_edges(instance.m_graph) == 6);

  // z3 test starts here
  z3::context ctx;
  z3::solver solver(ctx);
  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id);

  for (const auto& edge: ev_edges) {
    spdlog::info("signal: {}", edge.to_string());
  }

  ducode::DesignInstance::write_smt2(solver, "smt_qifa.smt2");

  for (uint32_t i = 0; i < 2; i++) {
    for (uint32_t j = 0; j < 2; j++) {
      solver.push();
      solver.add(ev_edges[0] == static_cast<int>(i));
      solver.add(ev_edges[1] == static_cast<int>(j));

      CHECK(solver.check() == z3::sat);
      spdlog::debug("{}", solver.to_smt2());
      z3::model solver_model = solver.get_model();

      // Break down nested conditional operator for better readability
      int expected_value = 0;
      if (i == 0) {
        expected_value = (j < 2) ? 0 : 3;
      } else {
        expected_value = 2;
      }
      spdlog::info("{} : {} == {}", ev_edges[4].to_string(), solver.get_model().eval(ev_edges[4]).as_int64(), expected_value);
      CHECK(solver.get_model().eval(ev_edges[4]).as_int64() == expected_value);

      solver.pop();
    }
  }
}

TEST_CASE("smt_qifa2", "[smt2]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "qifa_example" / "qifa_exampleSplitting.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  CHECK(boost::num_vertices(instance.m_graph) == 4);
  CHECK(boost::num_edges(instance.m_graph) == 3);

  // z3 test starts here
  z3::context ctx;
  z3::solver solver(ctx);
  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id);

  for (const auto& edge: ev_edges) {
    spdlog::info("signal: {}", edge.to_string());
  }

  ducode::DesignInstance::write_smt2(solver, "smt_qifa.smt2");

  for (uint32_t i = 0; i < 2; i++) {
    for (uint32_t j = 0; j < 2; j++) {
      solver.push();
      solver.add(ev_edges[0] == static_cast<int>(i));
      solver.add(ev_edges[1] == static_cast<int>(j));

      CHECK(solver.check() == z3::sat);
      spdlog::debug("{}", solver.to_smt2());
      z3::model solver_model = solver.get_model();

      // Break down nested conditional operator for better readability
      uint32_t expected_value = 0;
      expected_value = i ^ j;
      spdlog::info("{} : {} == {}", ev_edges[2].to_string(), solver.get_model().eval(ev_edges[2]).as_int64(), expected_value);
      CHECK(solver.get_model().eval(ev_edges[2]).as_int64() == expected_value);

      solver.pop();
    }
  }
}
TEST_CASE("smt_mux", "[smt2]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "basic_gates" / "mux" / "mux.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  CHECK(boost::num_vertices(instance.m_graph) == 5);
  CHECK(boost::num_edges(instance.m_graph) == 4);

  // z3 test starts here
  z3::context ctx;
  z3::solver solver(ctx);
  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id);


  for (uint32_t i = 0; i < 2; i++) {
    for (uint32_t j = 0; j < 2; j++) {
      for (uint32_t s = 0; s < 2; s++) {
        solver.push();
        solver.add(ev_edges[0] == static_cast<int>(i));
        solver.add(ev_edges[1] == static_cast<int>(j));
        solver.add(ev_edges[2] == static_cast<int>(s));

        CHECK(solver.check() == z3::sat);
        spdlog::debug("{}", solver.to_smt2());

        CHECK(solver.get_model().eval(ev_edges[3]).as_int64() == (s == 1 ? j : i));

        solver.pop();
      }
    }
  }
}

TEST_CASE("smt_pmux", "[smt2][.]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "flattened_verilog" / "pmux.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  CHECK(boost::num_vertices(instance.m_graph) == 10);
  CHECK(boost::num_edges(instance.m_graph) == 13);

  // z3 test starts here
  z3::context ctx;
  z3::solver solver(ctx);
  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id);


  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++) {
      solver.push();
      solver.add(ev_edges[0] == i);
      solver.add(ev_edges[1] == j);

      CHECK(solver.check() == z3::sat);
      spdlog::debug("{}", solver.get_model());
      solver.pop();
    }
  }
}

TEST_CASE("basic_smt2_multi_gate", "[smt2]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "multi_and" / "multi_and.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  CHECK(boost::num_vertices(instance.m_graph) == 7);
  CHECK(boost::num_edges(instance.m_graph) == 7);

  // z3 test starts here
  z3::context ctx;
  z3::solver solver(ctx);
  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id);


  for (uint32_t i = 0; i < 2; i++) {
    for (uint32_t j = 0; j < 2; j++) {
      for (uint32_t k = 0; k < 2; k++) {
        solver.push();
        solver.add(ev_edges[0] == static_cast<int>(i));
        solver.add(ev_edges[1] == static_cast<int>(j));
        solver.add(ev_edges[2] == static_cast<int>(k));

        CHECK(solver.check() == z3::sat);
        spdlog::debug("{}", solver.to_smt2());
        auto first_res = i & j;
        auto second_res = i & k;
        spdlog::debug("{} == {}", solver.get_model().eval(ev_edges[4]).as_int64(), (first_res & second_res));
        CHECK(solver.get_model().eval(ev_edges[4]).as_int64() == ((i & j) & (i & k)));
        solver.pop();
      }
    }
  }
}

TEST_CASE("basic_smt2_not_gate", "[smt2]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "not" / "not.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  CHECK(boost::num_vertices(instance.m_graph) == 5);
  CHECK(boost::num_edges(instance.m_graph) == 4);

  // z3 test starts here
  z3::context ctx;
  z3::solver solver(ctx);
  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id);


  for (uint32_t i = 0; i < 2; i++) {
    solver.push();
    solver.add(ev_edges[0] == static_cast<int>(i));

    CHECK(solver.check() == z3::sat);
    spdlog::debug("{}", solver.to_smt2());

    spdlog::debug("{} == {}", solver.get_model().eval(ev_edges[1]).as_int64(), (i == 0 ? 1 : 0));
    CHECK(solver.get_model().eval(ev_edges[1]).as_int64() == (i == 0 ? 1 : 0));
    solver.pop();
  }
}

TEST_CASE("basic_smt2_unary_gates", "[smt2]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "basic_gates" / "unary_gates" / "unary_gates_test.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  CHECK(boost::num_vertices(instance.m_graph) == 17);
  CHECK(boost::num_edges(instance.m_graph) == 16);

  // z3 test starts here
  z3::context ctx;
  z3::solver solver(ctx);
  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id);

  for (uint32_t i = 0; i < 2; i++) {
    solver.push();
    solver.add(ev_edges[0] == static_cast<int>(i));

    CHECK(solver.check() == z3::sat);


    spdlog::debug("{}", solver.get_model());
    solver.pop();
  }
}

TEST_CASE("basic_smt2_binary_gates", "[smt2][.]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "basic_gates" / "binary_gates" / "binary_gates_test.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  // CHECK(boost::num_vertices(instance.m_graph) == 3);
  // CHECK(boost::num_edges(instance.m_graph) == 2);

  // z3 test starts here
  z3::context ctx;
  z3::solver solver(ctx);
  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id);


  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++) {

      solver.push();
      solver.add(ev_edges[0] == i);
      solver.add(ev_edges[1] == j);

      CHECK(solver.check() == z3::sat);


      spdlog::debug("{}", solver.get_model());
      solver.pop();
    }
  }
}

// test case for split; one input uses only partial bits of the incoming edge
TEST_CASE("smt_split", "[smt2]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "split" / "split.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  CHECK(boost::num_vertices(instance.m_graph) == 4);
  CHECK(boost::num_edges(instance.m_graph) == 3);


  // z3 test starts here
  z3::context ctx;
  z3::solver solver(ctx);
  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id);


  for (int i = 0; i < 3; i++) {
    solver.push();
    solver.add(ev_edges[0] == i);

    CHECK(solver.check() == z3::sat);


    spdlog::debug("{}", solver.get_model());
    solver.pop();
  }
}

// test case for split_2; both inputs of an and gate only use partial bits of their respective incoming edges
TEST_CASE("smt_split2", "[smt2]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "split_2" / "split.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  CHECK(boost::num_vertices(instance.m_graph) == 4);
  CHECK(boost::num_edges(instance.m_graph) == 3);


  // z3 test starts here
  z3::context ctx;
  z3::solver solver(ctx);
  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id);


  for (int i = 0; i < 3; i++) {
    solver.push();
    solver.add(ev_edges[0] == i);

    CHECK(solver.check() == z3::sat);


    spdlog::debug("{}", solver.get_model());
    solver.pop();
  }
}

// Test case for split_3; one input consisting of bits from two incoming edges; order of bits important
TEST_CASE("smt_split3", "[smt2]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "split_3" / "split.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  CHECK(boost::num_vertices(instance.m_graph) == 4);
  CHECK(boost::num_edges(instance.m_graph) == 4);


  // z3 test starts here
  z3::context ctx;
  z3::solver solver(ctx);
  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id);


  const int32_t input_0 = 8;
  const int32_t input_1 = 3;

  solver.push();
  solver.add(ev_edges[0] == input_0);
  solver.add(ev_edges[1] == input_1);

  CHECK(solver.check() == z3::sat);
  CHECK(solver.get_model().eval(ev_edges[0]).as_int64() == 8);
  CHECK(solver.get_model().eval(ev_edges[1]).as_int64() == 3);
  CHECK(solver.get_model().eval(ev_edges[2]).as_int64() == 2);


  spdlog::debug("{}", solver.get_model());
  solver.pop();
}

// Test case for split_4; one input consisting of bits from two incoming edges; order of bits important
TEST_CASE("smt_split4", "[smt2]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "split_4" / "split.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  CHECK(boost::num_vertices(instance.m_graph) == 5);
  CHECK(boost::num_edges(instance.m_graph) == 6);


  // z3 test starts here
  z3::context ctx;
  z3::solver solver(ctx);
  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id);

  ducode::DesignInstance::write_smt2(solver, "smt_split4.smt2");

  const int32_t input_0 = 5;
  const int32_t input_1 = 1;

  solver.push();
  solver.add(ev_edges[0] == input_0);
  solver.add(ev_edges[1] == input_1);

  CHECK(solver.check() == z3::sat);

  //inputs
  CHECK(solver.get_model().eval(ev_edges[0]).as_int64() == 5);
  CHECK(solver.get_model().eval(ev_edges[1]).as_int64() == 1);
  CHECK(solver.get_model().eval(ev_edges[2]).as_int64() == 0);
  CHECK(solver.get_model().eval(ev_edges[4]).as_int64() == 1);


  spdlog::debug("{}", solver.get_model());
  solver.pop();
}

// Test case for split_4; one input consisting of bits from two incoming edges; order of bits important
TEST_CASE("smt_split5", "[smt2]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "split_5" / "split.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  CHECK(boost::num_vertices(instance.m_graph) == 5);
  CHECK(boost::num_edges(instance.m_graph) == 4);

  // z3 test starts here
  z3::context ctx;
  z3::solver solver(ctx);
  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id);


  const int32_t input_0 = 5;
  const int32_t input_1 = 2;

  solver.push();
  solver.add(ev_edges[0] == input_0);
  solver.add(ev_edges[1] == input_1);


  CHECK(solver.check() == z3::sat);

  //inputs
  CHECK(solver.get_model().eval(ev_edges[0]).as_int64() == 5);
  CHECK(solver.get_model().eval(ev_edges[1]).as_int64() == 2);

  spdlog::debug("{}", solver.get_model());
  solver.pop();
}

// Test case for split_6; one output of instantiated module consisting of bits from two incoming edges; order of bits important
TEST_CASE("smt_split6", "[smt2]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "split_6" / "split.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  CHECK(boost::num_vertices(instance.m_graph) == 6);
  CHECK(boost::num_edges(instance.m_graph) == 5);

  // z3 test starts here
  z3::context ctx;
  z3::solver solver(ctx);
  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id);


  const int32_t input_0 = 5;
  const int32_t input_1 = 2;

  solver.push();
  solver.add(ev_edges[0] == input_0);
  solver.add(ev_edges[1] == input_1);


  CHECK(solver.check() == z3::sat);

  //inputs
  CHECK(solver.get_model().eval(ev_edges[0]).as_int64() == 5);
  CHECK(solver.get_model().eval(ev_edges[1]).as_int64() == 2);

  spdlog::debug("{}", solver.get_model());
  solver.pop();
}

TEST_CASE("smt_hierarchy", "[smt2]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "hierarchy" / "hierarchy.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  CHECK(boost::num_vertices(instance.m_graph) == 29);
  CHECK(boost::num_edges(instance.m_graph) == 35);

  // z3 test starts here
  z3::context ctx;
  z3::solver solver(ctx);
  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id);

  ducode::DesignInstance::write_smt2(solver, "smt_hierarchy.smt2");

  solver.push();
  solver.add(ev_edges[0] == 1);// a
  solver.add(ev_edges[1] == 0);// d_0
  solver.add(ev_edges[4] == 1);// clk_1
  const int input_1 = 5;
  solver.add(ev_edges[input_1] == 1);// d_1

  CHECK(solver.check() == z3::sat);

  z3::model m = solver.get_model();
  spdlog::debug("{}", solver.get_model());
  solver.pop();
}

TEST_CASE("smt_hierarchy_small", "[smt2]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "hierarchy_small" / "split.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  CHECK(boost::num_vertices(instance.m_graph) == 4);
  CHECK(boost::num_edges(instance.m_graph) == 3);

  // z3 test starts here
  z3::context ctx;
  z3::solver solver(ctx);
  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id);


  solver.push();
  solver.add(ev_edges[0] == 1);// a
  solver.add(ev_edges[1] == 1);// d_0
  solver.add(ev_edges[2] == 1);// d_1

  CHECK(solver.check() == z3::sat);


  z3::model m = solver.get_model();
  spdlog::debug("{}", solver.get_model());
  solver.pop();
}

TEST_CASE("smt_constants", "[smt2]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "constants" / "constants_1.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  CHECK(boost::num_vertices(instance.m_graph) == 5);
  CHECK(boost::num_edges(instance.m_graph) == 4);

  // z3 test starts here
  z3::context ctx;
  z3::solver solver(ctx);
  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id);


  const int32_t input_0 = 5;
  const int32_t input_1 = 2;

  solver.push();
  solver.add(ev_edges[0] == input_0);
  solver.add(ev_edges[1] == input_1);


  CHECK(solver.check() == z3::sat);

  //inputs
  CHECK(solver.get_model().eval(ev_edges[0]).as_int64() == 5);
  CHECK(solver.get_model().eval(ev_edges[1]).as_int64() == 2);

  spdlog::debug("{}", solver.get_model());
  solver.pop();
}

TEST_CASE("smt_mem", "[smt2]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "mem_example" / "ram.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  CHECK(boost::num_vertices(instance.m_graph) == 13);
  CHECK(boost::num_edges(instance.m_graph) == 19);

  // z3 test starts here
  z3::context ctx;
  z3::solver solver(ctx);
  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  z3::params params(ctx);
  params.set("mbqi", true);
  solver.set(params);

  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, 0);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, 1);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, 2);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, 3);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, 4);

  ducode::DesignInstance::write_smt2(solver, "smt_mem.smt2");

  const int32_t input_clk = 1;
  const int32_t input_0 = 5;
  const int32_t input_1 = 2;
  const int32_t input_2 = 6;
  const int32_t input_3 = 7;

  const int32_t input_signal_0 = 12;
  const int32_t input_signal_1 = 13;
  const int32_t input_signal_2 = 14;
  const int32_t input_signal_3 = 24;
  const int32_t input_signal_4 = 34;
  const int32_t input_signal_5 = 44;
  const int32_t input_signal_6 = 10;
  const int32_t input_signal_7 = 20;
  const int32_t input_signal_8 = 30;
  const int32_t input_signal_9 = 40;
  const int32_t input_signal_10 = 50;
  const int32_t input_signal_11 = 9;

  // solver.add(ev_edges[0] == input_clk);
  solver.add(ev_edges[1] == input_0);
  solver.add(ev_edges[2] == input_1);
  solver.add(ev_edges[3] == 1);// write_enable signal set to 1
  solver.add(ev_edges[input_signal_1] == input_2);
  solver.add(ev_edges[input_signal_0] == input_3);
  solver.add(ev_edges[input_signal_2] == 1);
  solver.add(ev_edges[input_signal_3] == 0);
  solver.add(ev_edges[input_signal_4] == 0);
  solver.add(ev_edges[input_signal_5] == 0);

  // solver.push();
  bool sat = false;
  if (solver.check() == z3::sat) {
    sat = true;
    spdlog::debug("{}", solver.get_model());
  }
  CHECK(sat);

  //inputs
  CHECK(solver.get_model().eval(ev_edges[1]).as_int64() == input_0);
  CHECK(solver.get_model().eval(ev_edges[2]).as_int64() == input_1);

  CHECK(solver.get_model().eval(ev_edges[13]).as_int64() == input_2);
  const int32_t mem_size = 64;
  const int32_t mem_addr_size = 6;
  for (uint32_t i = 0; i < mem_size; ++i) {// Assuming the RAM has 64 memory locations
    spdlog::info("init_RAM[{}]: {}", i, solver.get_model().eval(select(ev_edges[input_signal_6], ctx.bv_val(static_cast<int>(i), mem_addr_size))).as_uint64());
  }
  for (uint32_t i = 0; i < mem_size; ++i) {// Assuming the RAM has 64 memory locations
    spdlog::info("t0_RAM[{}]: {}", i, solver.get_model().eval(select(ev_edges[input_signal_11], ctx.bv_val(static_cast<int>(i), mem_addr_size))).as_uint64());
  }

  for (uint32_t i = 0; i < mem_size; ++i) {// Assuming the RAM has 64 memory locations
    spdlog::info("t1_RAM[{}]: {}", i, solver.get_model().eval(select(ev_edges[input_signal_7], ctx.bv_val(static_cast<int>(i), mem_addr_size))).as_uint64());
  }

  for (uint32_t i = 0; i < mem_size; ++i) {// Assuming the RAM has 64 memory locations
    spdlog::info("t2_RAM[{}]: {}", i, solver.get_model().eval(select(ev_edges[input_signal_8], ctx.bv_val(static_cast<int>(i), mem_addr_size))).as_uint64());
  }

  for (uint32_t i = 0; i < mem_size; ++i) {// Assuming the RAM has 64 memory locations
    spdlog::info("t3_RAM[{}]: {}", i, solver.get_model().eval(select(ev_edges[input_signal_9], ctx.bv_val(static_cast<int>(i), mem_addr_size))).as_uint64());
  }

  for (uint32_t i = 0; i < mem_size; ++i) {// Assuming the RAM has 64 memory locations
    spdlog::info("t4_RAM[{}]: {}", i, solver.get_model().eval(select(ev_edges[input_signal_10], ctx.bv_val(static_cast<int>(i), mem_addr_size))).as_uint64());
  }

  for (const auto& edge: ev_edges) {
    spdlog::info("Edge {}: {}", edge.to_string(), solver.get_model().eval(edge).as_int64());
  }
  // solver.pop();
}

// the final round module as it is used in aes_128.v but number of instantiated modules reduced to reduce the number of memory locations
TEST_CASE("smt_aes_final_round", "[smt2][.]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "aes_128" / "final_round.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  //Z3_open_log("smt_aes_final_round.log");

  // z3 test starts here
  z3::context ctx;

  z3::tactic t(ctx, "default");
  z3::solver solver = t.mk_solver();
  z3::params params(ctx);
  params.set("mbqi", true);
  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  //Z3_global_param_set("verbose", "10");// Set verbosity level to 10
  solver.set(params);

  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, 0);

  //ducode::DesignInstance::write_smt2(solver, "smt_aes_final_round.smt2");

  const int32_t input_0 = 5;
  const int32_t input_1 = 2;

  CHECK(solver.check() == z3::sat);

  //Z3_close_log();

  spdlog::debug("{}", solver.get_model());
  // solver.pop();
}

// the final_round module complete own its own as it is used in aes_128.v
TEST_CASE("smt_aes_final_round_full", "[smt2][.]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "aes_128" / "final_round_full.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  //Z3_open_log("smt_aes_final_round_full.log");

  // z3 test starts here
  z3::context ctx;

  z3::tactic t(ctx, "default");
  z3::solver solver = t.mk_solver();
  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  z3::params params(ctx);
  params.set("mbqi", true);
  //Z3_global_param_set("verbose", "10");// Set verbosity level to 10
  solver.set(params);

  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, 0);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, 1);

  ducode::DesignInstance::write_smt2(solver, "smt_aes_final_round_full.smt2");

  const int32_t input_0 = 5;
  const int32_t input_1 = 2;

  CHECK(solver.check() == z3::sat);

  //Z3_close_log();

  spdlog::debug("{}", solver.get_model());
  // solver.pop();
}

TEST_CASE("smt_aes", "[smt2][.]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "aes_128" / "aes_128.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  //Z3_open_log("smt_aes.log");

  // z3 test starts here
  z3::context ctx;

  z3::tactic t(ctx, "default");
  z3::solver solver = t.mk_solver();
  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  z3::params params(ctx);
  params.set("mbqi", true);
  //Z3_global_param_set("verbose", "10");// Set verbosity level to 10
  solver.set(params);

  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, 0);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, 1);

  ducode::DesignInstance::write_smt2(solver, "smt_aes_final_round_full.smt2");

  const int32_t input_0 = 5;
  const int32_t input_1 = 2;

  CHECK(solver.check() == z3::sat);

  //Z3_close_log();

  spdlog::debug("{}", solver.get_model());
  // solver.pop();
}

TEST_CASE("smt_mips", "[smt2][.]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "mips_16_core_top" / "mips_16_core_top.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  //Z3_open_log("smt_mips.log");

  // z3 test starts here
  z3::context ctx;

  z3::tactic t(ctx, "default");
  z3::solver solver = t.mk_solver();
  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  z3::params params(ctx);
  params.set("mbqi", true);
  //Z3_global_param_set("verbose", "10");// Set verbosity level to 10
  solver.set(params);

  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, 0);
  // instance.rt_smt2(ctrtver, ev_edges, signal_name_to_edge_id, 1, 1);

  ducode::DesignInstance::write_smt2(solver, "smt_mips.smt2");

  const int32_t input_0 = 5;
  const int32_t input_1 = 2;

  CHECK(solver.check() == z3::sat);

  //Z3_close_log();

  spdlog::debug("{}", solver.get_model());
  // solver.pop();
}

TEST_CASE("smt_yadmc", "[smt2][.]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "yadmc" / "yadmc.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  //Z3_open_log("smt_yadmc.log");

  // z3 test starts here
  z3::context ctx;

  z3::tactic t(ctx, "default");
  z3::solver solver = t.mk_solver();
  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  z3::params params(ctx);
  params.set("mbqi", true);
  //Z3_global_param_set("verbose", "10");// Set verbosity level to 10
  solver.set(params);

  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, 0);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, 1);

  ducode::DesignInstance::write_smt2(solver, "smt_aes_final_round_full.smt2");

  const int32_t input_0 = 5;
  const int32_t input_1 = 2;

  CHECK(solver.check() == z3::sat);

  //Z3_close_log();

  spdlog::debug("{}", solver.get_model());
  // solver.pop();
}

TEST_CASE("smt_export", "[smt2][.]") {

  const auto* design_name = GENERATE("SPI_Master", "SPI_Slave", "picorv32", "y86", "simple_spi_top", "openMSP430");

  auto json_file = std::filesystem::path{TESTFILES_DIR} / design_name / fmt::format("{}.json", design_name);
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  //Z3_open_log(fmt::format("smt_{}.log", design_name).c_str());

  // z3 test starts here
  z3::context ctx;

  z3::tactic t(ctx, "default");
  z3::solver solver = t.mk_solver();
  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  //Z3_global_param_set("verbose", "10");// Set verbosity level to 10
  //Z3_global_param_set("mbqi", "true"); // Set verbosity level to 10

  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, 0);
  // instance.export2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, 1);

  ducode::DesignInstance::write_smt2(solver, fmt::format("smt_{}.smt2", design_name));

  CHECK(solver.check() == z3::sat);

  //Z3_close_log();

  spdlog::debug("{}", solver.get_model());
  // solver.pop();
}

TEST_CASE("smt_unrolling_export", "[smt2][.]") {

  const auto* design_name = GENERATE("SPI_Master", "SPI_Slave", "picorv32", "y86", "simple_spi_top", "openMSP430");

  spdlog::info("Starting Design {}", design_name);

  auto json_file = std::filesystem::path{TESTFILES_DIR} / design_name / fmt::format("{}.json", design_name);
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  //Z3_open_log(fmt::format("smt_{}.log", design_name).c_str());

  // z3 test starts here
  z3::context ctx;

  z3::tactic t(ctx, "default");
  z3::solver solver = t.mk_solver();
  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  //Z3_global_param_set("verbose", "10");// Set verbosity level to 10
  //Z3_global_param_set("mbqi", "true"); // Set verbosity level to 10

  const int32_t unroll_steps = 5;
  for (uint32_t i = 0; i < unroll_steps; ++i) {
    instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, i);
  }
  // instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, 0);

  ducode::DesignInstance::write_smt2(solver, fmt::format("smt_{}.smt2", design_name));

  CHECK(solver.check() == z3::sat);

  //Z3_close_log();

  spdlog::debug("{}", solver.get_model());
}

TEST_CASE("smt_usb2uart", "[smt2][.]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "usb2uart" / "usb2uart.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  //Z3_open_log("smt_usb2uart.log");

  // z3 test starts here
  z3::context ctx;

  z3::tactic t(ctx, "default");
  z3::solver solver = t.mk_solver();
  z3::expr_vector ev_edges(ctx);
  std::unordered_map<std::string, uint32_t> signal_name_to_edge_id;// maps input and output names to smt signal ids in ev_edges (id == index)

  z3::params params(ctx);
  params.set("mbqi", true);
  //Z3_global_param_set("verbose", "10");// Set verbosity level to 10
  solver.set(params);

  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, 0);
  instance.export_smt2(ctx, solver, ev_edges, signal_name_to_edge_id, 1, 1);

  ducode::DesignInstance::write_smt2(solver, "smt_usb2uart.smt2");

  const int32_t input_0 = 5;
  const int32_t input_1 = 2;

  CHECK(solver.check() == z3::sat);

  //Z3_close_log();

  spdlog::debug("{}", solver.get_model());
  // solver.pop();
}
