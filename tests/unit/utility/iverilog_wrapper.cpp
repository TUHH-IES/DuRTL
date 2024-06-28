/* SPDX-License-Identifier: Apache-2.0 */

#include <ducode/ids.hpp>
#include <ducode/utility/iverilog_wrapper.hpp>
#include <ducode/utility/parser_utility.hpp>

#include <catch2/catch_all.hpp>
#include <vcd-parser/VCDFileParser.hpp>

TEST_CASE("VCD comparison", "[iverilog_wrapper]") {
  auto vcd_reference = std::filesystem::path{TESTFILES_DIR} / "gateExample" / "gateExample.vcd";
  VCDFileParser parser;
  std::shared_ptr<VCDFile> vcd_reference_obj;
  spdlog::debug("Load reference VCD FILE: {}", vcd_reference.string());
  REQUIRE(std::filesystem::exists(vcd_reference));
  vcd_reference_obj = parser.parse_file(vcd_reference.string());
  CHECK(*vcd_reference_obj == *vcd_reference_obj);
}

TEST_CASE("simulate design", "[iverilog_wrapper]") {
  auto input_file = std::filesystem::path{TESTFILES_DIR} / "gateExample" / "gateExample.v";
  auto testbench_file = std::filesystem::path{TESTFILES_DIR} / "gateExample" / "gateExample_tb.v";

  std::shared_ptr<VCDFile> data = ducode::simulate_design(input_file, testbench_file);

  auto vcd_reference = std::filesystem::path{TESTFILES_DIR} / "gateExample" / "gateExample.vcd";
  VCDFileParser parser;
  std::shared_ptr<VCDFile> vcd_reference_obj;
  spdlog::debug("Load reference VCD FILE: {}", vcd_reference.string());
  REQUIRE(std::filesystem::exists(vcd_reference));
  vcd_reference_obj = parser.parse_file(vcd_reference.string());

  CHECK(*data == *vcd_reference_obj);
}

TEST_CASE("file_exceptions", "[iverilog_wrapper]") {
  auto input_file = std::filesystem::path{TESTFILES_DIR} / "iverilog_wrapper" / "missing_file.v";
  auto testbench_file = std::filesystem::path{TESTFILES_DIR} / "iverilog_wrapper" / "missing_file_tb.v";

  REQUIRE_THROWS_AS(ducode::simulate_design(input_file, testbench_file), std::runtime_error);

  input_file = std::filesystem::path{TESTFILES_DIR} / "iverilog_wrapper" / "no_vvp.v";
  testbench_file = std::filesystem::path{TESTFILES_DIR} / "iverilog_wrapper" / "no_vvp_tb.v";

  REQUIRE_THROWS_AS(ducode::simulate_design(input_file, testbench_file), std::runtime_error);

  input_file = std::filesystem::path{TESTFILES_DIR} / "iverilog_wrapper" / "no_vcd.v";
  testbench_file = std::filesystem::path{TESTFILES_DIR} / "iverilog_wrapper" / "no_vcd_tb.v";

  REQUIRE_THROWS_AS(ducode::simulate_design(input_file, testbench_file), std::runtime_error);
}