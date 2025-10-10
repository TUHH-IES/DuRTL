//
// Created by gianluca on 15/04/2025.
//

#include <common_definitions.hpp>
#include <ducode/utility/verilog_parser.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <fmt/core.h>

#include <utility>

TEST_CASE("verilog_parser", "[verilog_parser]") {
  auto test_case = GENERATE(
      std::pair("hierarchy", "t1"),
      std::pair("dff", "d1"),
      std::pair("picorv32", "uut"),
      std::pair("SPI_Master", "uut"),
      std::pair("SPI_Slave", "Slave"),
      std::pair("simple_mem", "dut"));

  auto filepath = boost::filesystem::path{TESTFILES_DIR} / test_case.first / fmt::format("{}_tb.v", test_case.first);
  auto result = ducode::get_instances_names(filepath.string(), test_case.first);
  REQUIRE(result.size() == 1);
  CHECK(result[0] == test_case.second);
}