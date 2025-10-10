#include <ducode/utility/smt.hpp>

#include "ducode/instantiation_graph_traits.hpp"
#include <boost/dynamic_bitset/dynamic_bitset.hpp>
#include <catch2/catch_test_macros.hpp>
#include <z3++.h>

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>


TEST_CASE("smt utility create smt name", "[smt utility]") {

  std::vector<std::string> hierarchy = {"top", "module1"};
  std::string signal1 = ducode::create_smt_signal_name(1, hierarchy, "signal_a", 0);

  CHECK(signal1 == "|D1_top_module1_signal_a_time_0|");
}

TEST_CASE("smt extend signal value vector", "[smt utility]") {
  const uint32_t test_size = 8;

  ducode::SignalValue value({ducode::SignalBit::BIT_0, ducode::SignalBit::BIT_1});
  ducode::SignalBitVector valuevector = ducode::extend_signal_value_vector(value.get_value_vector(), test_size);

  CHECK(valuevector == ducode::SignalBitVector{ducode::SignalBit::BIT_1, ducode::SignalBit::BIT_0, ducode::SignalBit::BIT_0, ducode::SignalBit::BIT_0, ducode::SignalBit::BIT_0, ducode::SignalBit::BIT_0, ducode::SignalBit::BIT_0, ducode::SignalBit::BIT_0});

  const uint32_t xtest_size = 4;

  ducode::SignalValue xvalue({ducode::SignalBit::BIT_X, ducode::SignalBit::BIT_X});
  ducode::SignalBitVector xvector = ducode::extend_signal_value_vector(xvalue.get_value_vector(), xtest_size);
  CHECK(xvector == ducode::SignalBitVector{ducode::SignalBit::BIT_X, ducode::SignalBit::BIT_X, ducode::SignalBit::BIT_X, ducode::SignalBit::BIT_X});
}

TEST_CASE("smt get signal value as bitset", "[smt utility]") {

  SECTION("value as bitset with 01") {
    const uint32_t test_size = 2;
    ducode::SignalValue value({ducode::SignalBit::BIT_0, ducode::SignalBit::BIT_1});
    boost::dynamic_bitset<> bitset = ducode::get_signal_value_as_bitset(value, test_size);

    REQUIRE(bitset.size() == 2);
    CHECK(bitset.at(0) == 1);
    CHECK(bitset.at(1) == 0);
  }

  SECTION("value as bitset with 01 and zero extend") {
    const uint32_t test_size = 4;
    ducode::SignalValue value({ducode::SignalBit::BIT_0, ducode::SignalBit::BIT_1});
    boost::dynamic_bitset<> bitset = ducode::get_signal_value_as_bitset(value, test_size);

    REQUIRE(bitset.size() == 4);
    CHECK(bitset.at(0) == 1);
    CHECK(bitset.at(1) == 0);
    CHECK(bitset.at(2) == 0);
    CHECK(bitset.at(3) == 0);
  }

  SECTION("value as bitset with REAL value type") {

    const uint32_t test_size = 4;
    ducode::SignalValue value(3);
    boost::dynamic_bitset<> bitset = ducode::get_signal_value_as_bitset(value, test_size);

    REQUIRE(bitset.size() == 4);
    CHECK(bitset.at(0) == 1);
    CHECK(bitset.at(1) == 1);
    CHECK(bitset.at(2) == 0);
    CHECK(bitset.at(3) == 0);
  }
}

TEST_CASE("smt compare bits", "[smt utility]") {

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
  const uint32_t SIGNAL_SHORT_BIT_WIDTH = 1;

  // Create a bitvector sort and add some expressions
  z3::sort bv_sort = context->bv_sort(SIGNAL_A_BIT_WIDTH);// 8-bit bitvector
  expr_vector->push_back(context->bv_const(signal1.c_str(), SIGNAL_A_BIT_WIDTH));
  expr_vector->push_back(context->bv_const(signal2.c_str(), SIGNAL_BC_BIT_WIDTH));
  expr_vector->push_back(context->bv_const(signal3.c_str(), SIGNAL_SHORT_BIT_WIDTH));

  solver->add((*expr_vector)[0] == 0);
  solver->add((*expr_vector)[2] == 0);
  solver->check();

  // Signal name to expr ID mapping
  std::unordered_map<std::string, uint32_t> signal_map;
  signal_map[signal1] = 0;
  signal_map[signal2] = 1;
  signal_map[signal3] = 2;

  std::vector<ducode::SignalIdentifier> input_names = {{.name = "a", .hierarchy = hierarchy}, {.name = "b", .hierarchy = hierarchy}};
  std::vector<ducode::SignalIdentifier> output_names = {{.name = "c", .hierarchy = hierarchy}};

  SECTION("compare_bits throws error when value type is SCALAR") {
    ducode::SignalValue value(ducode::SignalBit::BIT_0);
    z3::expr test_expr = (*expr_vector)[0];
    REQUIRE_THROWS_AS(ducode::compare_bits(*solver, test_expr, value, SIGNAL_A_BIT_WIDTH), std::runtime_error);
  }

  SECTION("compare_bits == false when value type is SCALAR") {
    z3::expr test_expr = (*expr_vector)[2];
    ducode::SignalValue value(ducode::SignalBit::BIT_1);
    CHECK(ducode::compare_bits(*solver, test_expr, value, SIGNAL_SHORT_BIT_WIDTH) == false);
  }

  SECTION("compare_bits == false when value type is VECTOR") {
    z3::expr test_expr = (*expr_vector)[0];
    ducode::SignalValue value({ducode::SignalBit::BIT_1, ducode::SignalBit::BIT_1});
    CHECK(ducode::compare_bits(*solver, test_expr, value, SIGNAL_A_BIT_WIDTH) == false);
  }
}
