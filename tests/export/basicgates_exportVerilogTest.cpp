/* SPDX-License-Identifier: Apache-2.0 */

#include <ducode/design.hpp>
#include <ducode/utility/iverilog_wrapper.hpp>

#include <boost/filesystem.hpp>
#include <catch2/catch_all.hpp>
#include <vcd-parser/VCDComparisons.hpp>
#include <vcd-parser/VCDFileParser.hpp>

#include <iostream>
#include <string>

auto basic_gates_test_body(const std::string& file_name) {
  nlohmann::json params;
  params["ift"] = false;

  auto json_file = boost::filesystem::path{TESTFILES_DIR} / (file_name + ".json");
  auto reference_tb = boost::filesystem::path{TESTFILES_DIR} / (file_name + "_tb.v");
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  auto design = ducode::Design::parse_json(json_file.string());

  auto vExportFile = temp_dir / "binary_gates_test_IFT.v";
  design.write_verilog(vExportFile, params);
  REQUIRE(boost::filesystem::exists(vExportFile));

  auto data = ducode::simulate_design(vExportFile, reference_tb);
  return data;
}

TEST_CASE("export binary_gates", "[ExportVerilogTest]") {
  auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / "binary_gates" / "binary_gates_test.vcd";

  auto data = basic_gates_test_body("binary_gates/binary_gates_test");

  VCDFileParser parser;
  const std::shared_ptr<VCDFile> vcd_reference_obj = parser.parse_file(vcd_reference_file.string());

  const bool result = (*data == *vcd_reference_obj);
  CHECK(result);
}

TEST_CASE("export unary_gates", "[ExportVerilogTest]") {
  auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / "unary_gates" / "unary_gates_test.vcd";

  auto data = basic_gates_test_body("unary_gates/unary_gates_test");

  VCDFileParser parser;
  const std::shared_ptr<VCDFile> vcd_reference_obj = parser.parse_file(vcd_reference_file.string());

  const bool result = (*data == *vcd_reference_obj);
  CHECK(result);
}