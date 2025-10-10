/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/module.hpp>
#include <ducode/signals_data_manager.hpp>
#include <ducode/testbench.hpp>
#include <ducode/utility/VCD_comparisons.hpp>
#include <ducode/utility/iverilog_wrapper.hpp>

#include "ducode/utility/VCD_utility.hpp"
#include <boost/filesystem/operations.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <fmt/core.h>
#include <nlohmann/json_fwd.hpp>
#include <spdlog/spdlog.h>
#include <vcd-parser/VCDFile.hpp>
#include <vcd-parser/VCDFileParser.hpp>
#include <vcd-parser/VCDTypes.hpp>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <tuple>

/**
 * @brief Performs a simulation test on a design.
 * 
 * This function takes a path to a JSON file and performs a simulation test on the design specified in the file.
 * It creates a temporary directory, exports the design to Verilog, and simulates the exported Verilog design using a reference testbench.
 * The simulation results are returned as data.
 * 
 * @param path The path to the JSON file containing the design.
 * @return The simulation data.
 */
namespace {
std::shared_ptr<VCDFile> test_simulation(const std::string& path) {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / (path + ".json");
  auto reference_tb = boost::filesystem::path{TESTFILES_DIR} / (path + "_tb.v");
  auto reference_input = boost::filesystem::path{TESTFILES_DIR} / (path + ".txt");
  auto json_dir = json_file.parent_path();
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  REQUIRE(created);
  auto exported_design = temp_dir / "exported_verilog.v";

  nlohmann::json params;
  params["ift"] = false;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;

  auto design = ducode::Design::parse_json(json_file);
  design.write_verilog(exported_design, params);

  std::shared_ptr<VCDFile> data;

  if (exists(reference_input)) {
    data = ducode::simulate_design(json_dir, exported_design, reference_tb, reference_input);
  } else {
    data = ducode::simulate_design(json_dir, exported_design, reference_tb, {});
  }

  return data;
}
}// namespace

TEST_CASE("Export multiple modules", "[export]") {
  auto test_parameters = GENERATE(
      std::make_tuple("export/moduleinstanceTest", "ins", ""),
      std::make_tuple("export/split/split", "a1", "")
      // std::make_tuple("picorv32/picorv32", "testbench", "_gate_level"),// using scope uut this test fails
      // std::make_tuple("y86/y86", "dut", "_gate_level"),
      // std::make_tuple("SPI_Master/SPI_Master", "uut", "_exported"),
      // std::make_tuple("multiple_nets/multiple_nets", "dut", ""),
      // std::make_tuple("simple_mem/simple_mem", "dut", ""),
      // std::make_tuple("eth_wishbone/eth_wishbone", "dut", ""),
      // std::make_tuple("aes_128_mem/aes_128_mem", "dut", ""),
      // std::make_tuple("yadmc_sdram16/yadmc_sdram16", "dut", ""),
      // std::make_tuple("output_assignment/output_assignment", "dut", ""),
      // std::make_tuple("eth_fifo/eth_fifo", "dut", ""),
      // std::make_tuple("SPI_Slave/SPI_Slave", "Slave", "_output_gate"),
      // std::make_tuple("mips_16_core_top/mips_16_core_top", "uut", "_gate_level"),
      // std::make_tuple("yadmc_dpram/yadmc_dpram_write", "dut", ""),
      // std::make_tuple("yadmc_dpram/yadmc_dpram_read", "dut", ""),
      // std::make_tuple("usb2uart/usb2uart", "usb2uart_tb", ""),
      // std::make_tuple("i2c_controller/i2c_controller", "i2c_controller_tb", "")
  );

  const auto& [path, scope_name, vcd_suffix] = test_parameters;

  spdlog::info("*********** Now testing {} ************", path);

  auto data = test_simulation(path);
  auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / fmt::format("{}{}.vcd", path, vcd_suffix);
  REQUIRE(boost::filesystem::exists(vcd_reference_file));
  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_reference_file.string());

  const auto& scope = data->get_scope(scope_name);
  const auto& scope_reference = vcd_reference->get_scope(scope_name);
  for (auto* scope_reference_signal: scope_reference.signals) {
    auto signal_it = std::ranges::find_if(scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    if (signal_it != scope.signals.end()) {
      CHECK(ducode::no_repetitions_compare(vcd_reference->get_signal_values(scope_reference_signal->hash), data->get_signal_values((*signal_it)->hash)));
    }
  }
}

TEST_CASE("export fft16", "[export]") {
  auto json_file = std::filesystem::path(TESTFILES_DIR) / "fft" / "fft16.json";
  auto vcd_file = boost::filesystem::path{TESTFILES_DIR} / "fft" / "fft16.vcd";

  auto design = ducode::Design::parse_json(json_file);
  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_file.string());
  REQUIRE(vcd_reference != nullptr);

  //for (const ducode::Module& module: design.get_modules()) {
  //module.write_graphviz(module.get_name() + ".dot");
  //}

  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  REQUIRE(created);

  nlohmann::json params;
  params["ift"] = false;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;

  auto exported_testbench = temp_dir / "testbench.v";
  auto exported_design = temp_dir / "test.v";
  design.write_verilog(exported_design, params);
  REQUIRE(boost::filesystem::exists(exported_design));
  const VCDScope* root_scope = &ducode::find_root_scope(vcd_reference, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_reference, root_scope);
  const ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));
  testbench.write_verilog(exported_testbench, params);
  REQUIRE(boost::filesystem::exists(exported_testbench));

  auto vcd_data = ducode::simulate_design(exported_design, exported_testbench);

  const auto& outer_scope = vcd_data->get_scope("fft16_tb");
  const auto& outer_scope_reference = vcd_reference->get_scope("fft16_tb");
  for (const auto& port: design.get_module("fft16").get_ports()) {
    spdlog::info(port.m_name);
    auto lambda = [&](const auto& signal) { return signal->reference == port.m_name; };
    auto hash_iter = std::ranges::find_if(outer_scope.signals, lambda);
    auto hash_reference_iter = std::ranges::find_if(outer_scope_reference.signals, lambda);
    REQUIRE(hash_iter != outer_scope.signals.end());
    REQUIRE(hash_reference_iter != outer_scope_reference.signals.end());

    auto signal_values = vcd_data->get_signal_values((*hash_iter)->hash);
    const auto& signal_values_reference = vcd_reference->get_signal_values((*hash_reference_iter)->hash);

    for (auto& element: signal_values) {
      if (element.value.get_type() == VCDValueType::VECTOR && element.value.get_value_vector().size() != port.m_bits.size()) {
        auto new_vector = element.value.get_value_vector();
        if (element.value.get_value_vector().size() == 1) {
          for (auto i = 0ul; i < port.m_bits.size() - element.value.get_value_vector().size(); ++i) {
            new_vector.emplace_back(element.value.get_value_vector().at(0));
          }
        } else if (element.value.get_value_vector().size() > 1) {
          for (auto i = 0ul; i < port.m_bits.size() - element.value.get_value_vector().size(); ++i) {
            new_vector.emplace(new_vector.cbegin(), VCDBit::VCD_0);
          }
        }
        element.value = VCDValue(new_vector);
      }
    }

    CHECK(signal_values == signal_values_reference);
  }
}

TEST_CASE("Export yadmc", "[export][.]") {
  auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / "yadmc" / "output.vcd";

  auto data = test_simulation("yadmc/yadmc");

  REQUIRE(boost::filesystem::exists(vcd_reference_file));

  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_reference_file.string());

  // input output simulation should be enough for such a complex design_file.
  const auto& outer_scope = data->get_scope("yadmc_test");
  const auto& outer_scope_reference = vcd_reference->get_scope("yadmc_test");
  for (auto* scope_reference_signal: outer_scope_reference.signals) {
    auto signal_it = std::ranges::find_if(outer_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    REQUIRE(signal_it != outer_scope.signals.end());
    spdlog::info("Signal: {}", scope_reference_signal->reference);
    CHECK(ducode::no_repetitions_compare(vcd_reference->get_signal_values(scope_reference_signal->hash), data->get_signal_values((*signal_it)->hash)));
  }
}

TEST_CASE("flipflop export", "[single_cell_export]") {

  const auto* ff_type = GENERATE("dff", "dffe", "aldff", "adff", "aldffe", "adffe", "sdff", "sdffe", "dlatch", "dffsr", "dffsre",
                                 "adlatch", "dlatchsr", "2bit_dffsr", "2bit_dffsre", "sdffe_en_polarity", "dlatchsr_1bit");

  spdlog::info(ff_type);
  auto design_file_path = boost::filesystem::path{"flipFlops"} / ff_type / ff_type;
  auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / (design_file_path.string() + ".vcd");
  REQUIRE(boost::filesystem::exists(vcd_reference_file));
  auto data = test_simulation(design_file_path.string());

  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_reference_file.string());

  const bool result = (*data == *vcd_reference);
  CHECK(result);
}

TEST_CASE("cell export", "[single_cell_export]") {

  const auto* cell_type = GENERATE("shift_a_sign", "shift_b_sign", "shift_ab_sign", "shift_no_sign",
                                   "shiftx", "shiftx_no_sign", "sr", "pmux", "pmux_ue", "mux");

  spdlog::info(cell_type);
  auto design_file_path = boost::filesystem::path{"test_cells"} / cell_type / cell_type;
  auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / (design_file_path.string() + ".vcd");
  REQUIRE(boost::filesystem::exists(vcd_reference_file));

  auto data = test_simulation(design_file_path.string());

  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_reference_file.string());

  const bool result = (*data == *vcd_reference);
  CHECK(result);
}

TEST_CASE("export binary_gates", "[ExportVerilogTest]") {
  const auto* gates_folder = GENERATE(
      "basic_gates/unary_gates/unary_gates_test",
      "basic_gates/binary_gates/binary_gates_test");

  auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / (std::string{gates_folder} + ".vcd");
  REQUIRE(boost::filesystem::exists(vcd_reference_file));

  auto data = test_simulation(gates_folder);

  VCDFileParser parser;
  const std::shared_ptr<VCDFile> vcd_reference_obj = parser.parse_file(vcd_reference_file.string());

  const bool result = (*data == *vcd_reference_obj);
  CHECK(result);
}
