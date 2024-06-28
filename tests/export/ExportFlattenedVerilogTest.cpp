/* SPDX-License-Identifier: Apache-2.0 */

#include <ducode/instantiation_graph.hpp>
#include <ducode/utility/iverilog_wrapper.hpp>
#include <ducode/utility/parser_utility.hpp>

#include <catch2/catch_all.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/unique.hpp>
#include <spdlog/spdlog.h>
#include <vcd-parser/VCDComparisons.hpp>

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
  instance.write_graphviz("graph.dot");

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

    auto design = ducode::Design::parse_json(json_file.string());

    VCDFileParser parser;
    auto vcd_reference = parser.parse_file(vcd_reference_file.string());
    auto instance = ducode::DesignInstance::create_instance(design);
    auto exported_design = temp_dir / ("flatten.v");
    instance.write_verilog(exported_design.string());
    instance.write_graphviz("graph.dot");

    auto data = ducode::simulate_design(exported_design, reference_tb, reference_input);

    const auto& inner_scope = data->get_scope(fmt::format("main", design_file));
    const auto& inner_scope_reference = vcd_reference->get_scope(fmt::format("main", design_file));
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
      CHECK(signal_it != inner_scope.signals.end());
      auto reference_values = vcd_reference->get_signal_values(scope_reference_signal->hash) |
                              ranges::views::transform([](const VCDTimedValue& value) { return value.value; }) |
                              ranges::views::unique;
      auto values = data->get_signal_values((*signal_it)->hash) |
                    ranges::views::transform([](const VCDTimedValue& value) { return value.value; }) |
                    ranges::views::unique;

      auto ref_it = reference_values.begin();
      auto val_it = values.begin();
      for (; ref_it != reference_values.end() || val_it != values.end(); ref_it++, val_it++) {
        if (*ref_it != *val_it) {
          CHECK(false);
        }
      }
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
    CHECK(signal_it != inner_scope.signals.end());
    auto reference_values = vcd_reference->get_signal_values(scope_reference_signal->hash) |
                            ranges::views::transform([](const VCDTimedValue& value) { return value.value; }) |
                            ranges::views::unique;
    auto values = data->get_signal_values((*signal_it)->hash) |
                  ranges::views::transform([](const VCDTimedValue& value) { return value.value; }) |
                  ranges::views::unique;

    auto ref_it = reference_values.begin();
    auto val_it = values.begin();
    for (; ref_it != reference_values.end() || val_it != values.end(); ref_it++, val_it++) {
      if (*ref_it != *val_it) {
        CHECK(false);
      }
    }
  }
}