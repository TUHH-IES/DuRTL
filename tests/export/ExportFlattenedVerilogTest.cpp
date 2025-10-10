/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/instantiation_graph.hpp>
#include <ducode/utility/VCD_comparisons.hpp>
#include <ducode/utility/iverilog_wrapper.hpp>

#include <boost/filesystem/operations.hpp>
#include <catch2/catch_test_macros.hpp>
#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <vcd-parser/VCDFileParser.hpp>

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

auto export_flattened_test_function(const std::string& design_file_path) {
  auto json_file = std::filesystem::path{TESTFILES_DIR} / (design_file_path + ".json");
  auto reference_v = boost::filesystem::path{TESTFILES_DIR} / (design_file_path + ".v");
  auto reference_tb = boost::filesystem::path{TESTFILES_DIR} / (design_file_path + "_tb.v");
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  const auto design = ducode::Design::parse_json(json_file);

  auto instance = ducode::DesignInstance::create_instance(design);
  auto exported_design = temp_dir / "flatten.v";
  instance.write_verilog(exported_design);
  //instance.write_graphviz("graph.dot");

  auto data = ducode::simulate_design(exported_design, reference_tb);
  return data;
}

TEST_CASE("export_flattened_verilog", "[ExportFlattenedVerilog]") {
  auto data = export_flattened_test_function("flattened_verilog/hierarchyVector");

  auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / "flattened_verilog" / "hierarchyVector.vcd";
  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_reference_file.string());
  const auto& inner_scope = data->get_scope("hierarchyVector_tb");
  const auto& inner_scope_reference = vcd_reference->get_scope("hierarchyVector_tb");
  for (auto* scope_reference_signal: inner_scope_reference.signals) {
    auto signal_it = std::ranges::find_if(inner_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    CHECK(signal_it != inner_scope.signals.end());
    CHECK(vcd_reference->get_signal_values(scope_reference_signal->hash) == data->get_signal_values((*signal_it)->hash));
  }
}

TEST_CASE("export_flattened_dff_design", "[ExportFlattenedVerilog]") {
  auto data = export_flattened_test_function("dff_design/dff_design");

  auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / "dff_design" / "dff_design.vcd";
  auto reference_file = boost::filesystem::path{TESTFILES_DIR} / "dff_design" / "dff_design.v";
  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / "dff_design" / "dff_design_tb.v";

  auto reference_data = ducode::simulate_design(reference_file, testbench_file);

  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_reference_file.string());
  const auto& inner_scope = data->get_scope("dff_design_tb");
  const auto& inner_scope_reference = reference_data->get_scope("dff_design_tb");
  for (auto* scope_reference_signal: inner_scope_reference.signals) {
    auto signal_it = std::ranges::find_if(inner_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    CHECK(signal_it != inner_scope.signals.end());
    CHECK(reference_data->get_signal_values(scope_reference_signal->hash) == data->get_signal_values((*signal_it)->hash));
  }
}

TEST_CASE("export_flattened_verilog_edgecases", "[ExportFlattenedVerilog]") {

  const std::vector<std::pair<std::string, std::string>> designs = {
      {"constants/constants", "constants_tb"},
      {"mixed_inputs/mixed_inputs", "mixed_inputs_tb"}};

  for (const auto& design: designs) {
    auto data = export_flattened_test_function(design.first);
    auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / (design.first + ".vcd");
    VCDFileParser parser;
    auto vcd_reference = parser.parse_file(vcd_reference_file.string());
    const auto& inner_scope = data->get_scope(design.second);
    const auto& inner_scope_reference = vcd_reference->get_scope(design.second);
    for (auto* scope_reference_signal: inner_scope_reference.signals) {
      auto signal_it = std::ranges::find_if(inner_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
      CHECK(signal_it != inner_scope.signals.end());
      CHECK(vcd_reference->get_signal_values(scope_reference_signal->hash) == data->get_signal_values((*signal_it)->hash));
    }
  }
}

TEST_CASE("export_flattened_y86", "[ExportFlattenedVerilog]") {

  const std::vector<std::string> design_files = {"y86"};

  for (const auto& design_file: design_files) {

    auto json_file = std::filesystem::path{TESTFILES_DIR} / design_file / (design_file + ".json");
    auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / design_file / "y86_gate_level.vcd";
    auto reference_tb = boost::filesystem::path{TESTFILES_DIR} / design_file / (design_file + "_tb.v");
    auto reference_input = boost::filesystem::path{TESTFILES_DIR} / design_file / "y86.txt";
    auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
    auto created = boost::filesystem::create_directories(temp_dir);
    assert(created);

    auto design = ducode::Design::parse_json(json_file);

    VCDFileParser parser;
    auto vcd_reference = parser.parse_file(vcd_reference_file.string());
    auto instance = ducode::DesignInstance::create_instance(design);
    auto exported_design = temp_dir / ("flatten.v");
    instance.write_verilog(exported_design);
    //instance.write_graphviz("graph.dot");

    auto data = ducode::simulate_design(exported_design, reference_tb, reference_input);

    const auto& inner_scope = data->get_scope(fmt::format("y86_tb", design_file));
    const auto& inner_scope_reference = vcd_reference->get_scope(fmt::format("y86_tb", design_file));
    for (auto* scope_reference_signal: inner_scope_reference.signals) {
      spdlog::info("{}", scope_reference_signal->reference);
      auto signal_it = std::ranges::find_if(inner_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
      CHECK(signal_it != inner_scope.signals.end());
      CHECK(vcd_reference->get_signal_values(scope_reference_signal->hash) == data->get_signal_values((*signal_it)->hash));
    }
  }
}

TEST_CASE("export_flattened_verilog_basicGates", "[ExportFlattenedVerilog]") {

  const std::vector<std::string>
      design_files = {
          "andGate", "orGate", "xorGate", "andVector", "mux", "logic_andGate", "logic_orGate", "LeCell",
          "GeCell", "EqCell", "LtCell", "GtCell", "reduceAndGate", "reduceOrGate",
          "reduceXorGate", "reduceXnorGate", "posGate"};

  for (const auto& design_file: design_files) {
    auto data = export_flattened_test_function("flattened_verilog/" + design_file);

    auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / "flattened_verilog" / (design_file + ".vcd");
    VCDFileParser parser;
    auto vcd_reference = parser.parse_file(vcd_reference_file.string());
    const auto& inner_scope = data->get_scope(design_file + "_tb");
    const auto& inner_scope_reference = vcd_reference->get_scope(design_file + "_tb");
    for (auto* scope_reference_signal: inner_scope_reference.signals) {
      spdlog::info("{}", scope_reference_signal->reference);
      auto signal_it = std::ranges::find_if(inner_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
      REQUIRE(signal_it != inner_scope.signals.end());
      CHECK(ducode::no_repetitions_compare(vcd_reference->get_signal_values(scope_reference_signal->hash), data->get_signal_values((*signal_it)->hash)));
    }
  }
}

TEST_CASE("export_flattened_picorv32", "[ExportFlattenedVerilog]") {
  auto data = export_flattened_test_function("picorv32/picorv32");

  auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / "picorv32" / "picorv32.vcd";
  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_reference_file.string());
  const auto& inner_scope = data->get_scope("testbench");
  const auto& inner_scope_reference = vcd_reference->get_scope("testbench");
  for (auto* scope_reference_signal: inner_scope_reference.signals) {
    spdlog::info("{}", scope_reference_signal->reference);
    auto signal_it = std::ranges::find_if(inner_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    REQUIRE(signal_it != inner_scope.signals.end());
    CHECK(ducode::no_repetitions_compare(vcd_reference->get_signal_values(scope_reference_signal->hash), data->get_signal_values((*signal_it)->hash)));
  }
}

TEST_CASE("export_flattened_usb2uart", "[ExportFlattenedVerilog][.]") {
  auto data = export_flattened_test_function("usb2uart/usb2uart");

  auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / "usb2uart" / "usb2uart.vcd";
  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_reference_file.string());
  const auto& inner_scope = data->get_scope("usb2uart_tb");
  const auto& inner_scope_reference = vcd_reference->get_scope("usb2uart_tb");
  for (auto* scope_reference_signal: inner_scope_reference.signals) {
    spdlog::info("{}", scope_reference_signal->reference);
    auto signal_it = std::ranges::find_if(inner_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    REQUIRE(signal_it != inner_scope.signals.end());
    CHECK(ducode::no_repetitions_compare(vcd_reference->get_signal_values(scope_reference_signal->hash), data->get_signal_values((*signal_it)->hash)));
  }
}
