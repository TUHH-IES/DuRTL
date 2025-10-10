#include <ducode/signals_data_manager.hpp>

#include "ducode/instantiation_graph_traits.hpp"
#include "ducode/utility/smt.hpp"
#include <catch2/catch_test_macros.hpp>
#include <vcd-parser/VCDTypes.hpp>
#include <z3++.h>

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

TEST_CASE("SMTSignalsDataManager::get_signal retrieves correct signal info", "[signals_data_manager]") {
  // Setup context and solver
  auto context = std::make_shared<z3::context>();
  auto solver = std::make_shared<z3::solver>(*context);

  // Create signal expr vector
  auto expr_vector = std::make_shared<z3::expr_vector>(*context);

  std::vector<std::string> hierarchy = {"top", "module1"};
  std::string signal1 = ducode::create_smt_signal_name(1, hierarchy, "signal_a", 0);
  std::string signal2 = ducode::create_smt_signal_name(1, hierarchy, "signal_b", 0);
  std::string signal3 = ducode::create_smt_signal_name(1, hierarchy, "signal_c", 0);

  const uint32_t SIGNAL_A_BIT_WIDTH = 8;
  const uint32_t SIGNAL_BC_BIT_WIDTH = 16;

  // Create a bitvector sort and add some expressions
  z3::sort bv_sort = context->bv_sort(SIGNAL_A_BIT_WIDTH);// 8-bit bitvector
  expr_vector->push_back(context->bv_const(signal1.c_str(), SIGNAL_A_BIT_WIDTH));
  expr_vector->push_back(context->bv_const(signal2.c_str(), SIGNAL_BC_BIT_WIDTH));
  expr_vector->push_back(context->bv_const(signal3.c_str(), SIGNAL_BC_BIT_WIDTH));

  // Signal name to expr ID mapping
  std::unordered_map<std::string, uint32_t> signal_map;
  signal_map[signal1] = 0;
  signal_map[signal2] = 1;
  signal_map[signal3] = 2;

  std::vector<ducode::SignalIdentifier> input_names = {{.name = "a", .hierarchy = hierarchy}, {.name = "b", .hierarchy = hierarchy}};
  std::vector<ducode::SignalIdentifier> output_names = {{.name = "c", .hierarchy = hierarchy}};
  std::string top_module_name = "top";
  // Create the tracker
  ducode::SMTSignalsDataManager tracker(
      context, solver, signal_map, 1, expr_vector, input_names, output_names, top_module_name);

  SECTION("Get signal info for existing signal") {
    // Create a signal identifier
    ducode::SignalIdentifier signal_id;
    signal_id.name = "signal_a";
    signal_id.hierarchy = {"top", "module1"};

    // Get signal info
    auto signal_info = tracker.get_signal(signal_id);

    // Check the signal info
    REQUIRE(signal_info.reference == "signal_a");
    REQUIRE(signal_info.size == 8);
    REQUIRE(signal_info.hash == "top_module1_signal_a");
  }

  SECTION("Get signal info for another existing signal") {
    ducode::SignalIdentifier signal_id;
    signal_id.name = "signal_b";
    signal_id.hierarchy = {"top", "module1"};

    auto signal_info = tracker.get_signal(signal_id);

    REQUIRE(signal_info.reference == "signal_b");
    REQUIRE(signal_info.size == 16);
    REQUIRE(signal_info.hash == "top_module1_signal_b");
  }

  SECTION("Exception thrown for non-existent signal") {
    ducode::SignalIdentifier signal_id;
    signal_id.name = "non_existent_signal";
    signal_id.hierarchy = {"top", "module1"};

    REQUIRE_THROWS_AS(tracker.get_signal(signal_id), std::out_of_range);
  }
}

TEST_CASE("SMTSignalsDataManager not implemented functions throw errors as expected", "[signals_data_manager]") {

  ducode::SignalIdentifier signal_id = {.name = "signal_a", .hierarchy = {"top", "m1"}};
  ducode::SMTSignalsDataManager manager;

  REQUIRE_THROWS_AS(manager.get_tag_values(signal_id), std::runtime_error);
  REQUIRE_THROWS_AS(manager.get_tag_count_signal("", {}), std::runtime_error);
  REQUIRE_THROWS_AS(manager.get_tag_count_signal_to_signal(signal_id, signal_id), std::runtime_error);
  REQUIRE_THROWS_AS(manager.get_tag_sources(signal_id), std::runtime_error);
  REQUIRE_THROWS_AS(manager.is_empty(), std::runtime_error);
  REQUIRE_THROWS_AS(manager.get_stepsize("clk"), std::runtime_error);
  REQUIRE_THROWS_AS(manager.get_information_flow_source_signals_for_target_signal({}, signal_id), std::runtime_error);
  REQUIRE_THROWS_AS(manager.get_information_flow_source_signals_for_target_signal_at_timestep({}, signal_id, 1), std::runtime_error);
  VCDBitVector vec1;
  REQUIRE_THROWS_AS(manager.bitwise_vcd_compare(vec1, vec1), std::runtime_error);
  REQUIRE_THROWS_AS(manager.get_vcd_data_size(), std::runtime_error);
}

TEST_CASE("SMTSignalDataManager simple functions test", "[signal_data_manager]") {

  ducode::SignalIdentifier signal_id = {.name = "signal_a", .hierarchy = {"top", "m1"}};
  ducode::SMTSignalsDataManager manager;

  CHECK(manager.get_size() == 0);
}
