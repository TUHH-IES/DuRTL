/* SPDX-License-Identifier: Apache-2.0 */

#include <ducode/design.hpp>
#include <ducode/utility/VCD_comparisons.hpp>
#include <ducode/utility/ift_signals_contain_x.hpp>
#include <ducode/utility/iverilog_wrapper.hpp>

#include <boost/filesystem.hpp>
#include <catch2/catch_all.hpp>
#include <spdlog/spdlog.h>
#include <vcd-parser/VCDFileParser.hpp>

#include <string>

/**
 * \brief Tests for the export_verilog with IFT shadow logic. For FlipFlop, Latch cells and other more complex Cells,
 * reads in Json output verilog ( \c ift = true )
 * compares vcd of reference tagged testbench with reference tagged verilog file and exported tagged verilog file
*/

void cell_ift_test_function(const std::string& design_file_path) {
  nlohmann::json params;
  params["ift"] = true;

  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "ift" / (design_file_path + ".json");
  auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / "ift" / (design_file_path + "_ift.vcd");
  auto reference_tb = boost::filesystem::path{TESTFILES_DIR} / "ift" / (design_file_path + "_ift_tb.v");
  auto reference_input_tags = boost::filesystem::path{TESTFILES_DIR} / "ift" / (design_file_path + "_tag_input.txt");
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  std::string top_module = "m_0";
  auto design = ducode::Design::parse_json(json_file);
  design.set_top_module_name(top_module);

  auto vExportFile = temp_dir / "cell_IFT.v";

  design.write_verilog(vExportFile, params);
  REQUIRE(boost::filesystem::exists(vExportFile));

  auto data = ducode::simulate_design(vExportFile, reference_tb, reference_input_tags);

  VCDFileParser parser;
  std::shared_ptr<VCDFile> vcd_reference_obj = parser.parse_file(vcd_reference_file.string());

  CHECK(ducode::no_repetitions_compare(*data, *vcd_reference_obj));
  CHECK_FALSE(ducode::ift_signals_contain_x(*data, design));
}

TEST_CASE("flipflop_ift_test", "[single_cell_ift_test]") {

  const auto* ff_type = GENERATE(
      "dff", "dffe", "aldff", "adff", "aldffe", "adffe", "sdff", "sdffe", "dlatch", "adlatch", "dffsr", "dffsre",
      "dlatchsr", "mbit_dffsr", "mbit_dffsre", "sdffe_en_polarity", "dlatchsr_1bit");

  spdlog::info(ff_type);
  auto design_file_path = boost::filesystem::path{"flipFlops"} / ff_type / ff_type;
  cell_ift_test_function(design_file_path.string());
}

TEST_CASE("cell_ift_test", "[single_cell_ift_test]") {

  const auto* cell_type = GENERATE("shift", "sr", "pmux", "pmux_ue", "mux", "shiftx");

  spdlog::info(cell_type);
  auto design_file_path = boost::filesystem::path{"test_cells"} / cell_type / cell_type;
  cell_ift_test_function(design_file_path.string());
}