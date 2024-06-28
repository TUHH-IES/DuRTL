/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/utility/iverilog_wrapper.hpp>

#include <boost/filesystem/operations.hpp>
#include <catch2/catch_all.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <cassert>
#include <string>
#include <tuple>
#include <vector>

/**
 * \brief Tests for the export_verilog (without IFT shadow logic) Functions for FlipFlop, Latch cells and other more complex Cells,
 * reads in Json output verilog ( \c iftActive disabled )
 * compares vcd of testBench with original verilog file and exported verilog file
*/

auto cell_export_test_function(const std::string& design_file_path) {
  nlohmann::json params;
  params["ift"] = false;
  //file paths
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / (design_file_path + ".json");
  auto reference_tb = boost::filesystem::path{TESTFILES_DIR} / (design_file_path + "_tb.v");
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  const std::string top_module = "m_0";
  auto design = ducode::Design::parse_json(json_file.string());
  design.set_top_module_name(top_module);

  spdlog::debug("temp_dir: {}", temp_dir.string());
  auto vExportFile = temp_dir / "export_test.v";
  design.write_verilog(vExportFile, params);
  REQUIRE(boost::filesystem::exists(vExportFile));

  auto data = ducode::simulate_design(vExportFile, reference_tb);
  return std::make_tuple(data, design);
}

TEST_CASE("flipflop export", "[single_cell_export]") {

  const std::vector<std::string> ff_types = {"dff", "dffe", "aldff", "adff", "aldffe", "adffe", "sdff", "sdffe", "dlatch", "dffsr", "dffsre",
                                             "adlatch", "dlatchsr", "2bit_dffsr", "2bit_dffsre", "sdffe_en_polarity", "dlatchsr_1bit"};

  for (const auto& ff_type: ff_types) {
    spdlog::info(ff_type);
    auto design_file_path = boost::filesystem::path{"flipFlops"} / ff_type / ff_type;
    auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / (design_file_path.string() + ".vcd");
    auto [data, design] = cell_export_test_function(design_file_path.string());
    spdlog::debug("Load reference VCD FILE: {}", vcd_reference_file.string());
    REQUIRE(boost::filesystem::exists(vcd_reference_file));

    VCDFileParser parser;
    auto vcd_reference = parser.parse_file(vcd_reference_file.string());

    spdlog::debug("Comparing vcd-files from original design simulation {} and exported design simulation", vcd_reference_file.string());
    const bool result = (*data == *vcd_reference);
    CHECK(result);
  }
}

TEST_CASE("cell export", "[single_cell_export]") {

  const std::vector<std::string> cell_types = {"shift_a_sign", "shift_b_sign", "shift_ab_sign", "shift_no_sign",
                                               "shiftx", "shiftx_no_sign", "sr", "pmux", "pmux_ue", "mux"};

  nlohmann::json params;
  params["ift"] = false;

  for (const auto& cell_type: cell_types) {
    spdlog::info(cell_type);
    auto design_file_path = boost::filesystem::path{"test_cells"} / cell_type / cell_type;
    auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / (design_file_path.string() + ".vcd");
    auto [data, design] = cell_export_test_function(design_file_path.string());
    spdlog::debug("Load reference VCD FILE: {}", vcd_reference_file.string());
    REQUIRE(boost::filesystem::exists(vcd_reference_file));

    VCDFileParser parser;
    auto vcd_reference = parser.parse_file(vcd_reference_file.string());

    spdlog::debug("Comparing vcd-files from original design simulation {} and exported design simulation", vcd_reference_file.string());
    const bool result = (*data == *vcd_reference);
    CHECK(result);
  }
}
