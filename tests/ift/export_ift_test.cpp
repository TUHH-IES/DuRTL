/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/instantiation_graph.hpp>
#include <ducode/testbench.hpp>
#include <ducode/utility/VCD_comparisons.hpp>
#include <ducode/utility/iverilog_wrapper.hpp>

#include <boost/filesystem.hpp>
#include <catch2/catch_all.hpp>
#include <spdlog/spdlog.h>

#include <string>

///////////////////////////////////////////////////////////////////////////////////////////////
// Tests if the design was written correctly as an extended IFT version and
// also if the exported versions show the same behavior to the original verilog code
///////////////////////////////////////////////////////////////////////////////////////////////


std::tuple<ducode::Design, std::shared_ptr<VCDFile>, std::shared_ptr<VCDFile>> simulate_with_IFT(const std::string& file_name) {
  nlohmann::json params;
  params["ift"] = true;

  auto json_file = boost::filesystem::path{TESTFILES_DIR} / file_name / (file_name + ".json");
  auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / file_name / (file_name + ".vcd");
  auto reference_tb = boost::filesystem::path{TESTFILES_DIR} / file_name / (file_name + "_tb.v");
  auto reference_input_tags = boost::filesystem::path{TESTFILES_DIR} / file_name / (file_name + ".txt");
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  REQUIRE(created);
  auto verilog_export_file = temp_dir / "ift_design.v";

  auto design = ducode::Design::parse_json(json_file.string());

  spdlog::debug("temp_dir: {}", temp_dir.string());

  design.write_verilog(verilog_export_file, params);
  REQUIRE(boost::filesystem::exists(verilog_export_file));

  auto data = ducode::simulate_design(verilog_export_file, reference_tb, reference_input_tags);

  spdlog::debug("Load reference VCD FILE: {}", vcd_reference_file.string());

  VCDFileParser parser;
  auto vcd_reference_obj = parser.parse_file(vcd_reference_file.string());

  return {design, data, vcd_reference_obj};
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tests if the design was written back correctly as a verilog design that shows the same behavior to the original verilog code
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TEST_CASE("binary_gates_ift_test", "[ift_test]") {
  auto [design, simulation, reference] = simulate_with_IFT("binary_gates_ift_test");
  CHECK(*simulation == *reference);
}

TEST_CASE("unary_gates_ift_test", "[ift_test]") {
  auto [design, simulation, reference] = simulate_with_IFT("unary_gates_ift_test");
  CHECK(*simulation == *reference);
}

TEST_CASE("hierarchy_ift", "[ift_test]") {
  auto [design, simulation, reference] = simulate_with_IFT("hierarchy_ift");

  REQUIRE_FALSE(simulation->root_scope->children.empty());
  REQUIRE_FALSE(reference->root_scope->children.empty());

  auto* simulation_tb_scope = simulation->root_scope->children[0];
  auto* reference_tb_scope = reference->root_scope->children[0];

  CHECK(*simulation_tb_scope == *reference_tb_scope);

  bool contains_x = false;

  auto instance = ducode::DesignInstance::create_instance(design);
  instance.add_vcd_data(simulation);
  instance.write_graphviz("hierarchy.dot");
  for (auto [edge_it, edge_end] = boost::edges(instance.m_graph); edge_it != edge_end; ++edge_it) {
    if (instance.m_graph[*edge_it].net_ptr != nullptr) {
      auto tag_values = instance.get_tag_values(*edge_it);
      for (const auto* tag_vector: tag_values) {
        for (const auto& tag_value: *tag_vector) {
          for (const auto& tag_bit: tag_value.value.get_value_vector()) {
            if (tag_bit == VCDBit::VCD_X) {
              contains_x = true;
              break;
            }
          }
        }
      }
    }
  }

  CHECK_FALSE(contains_x);
}

TEST_CASE("y86_ift", "[ift_test]") {
  nlohmann::json params;
  params["ift"] = true;

  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "y86" / "y86.json";
  auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / "y86" / "y86_gate_level.vcd";
  auto reference_tb = boost::filesystem::path{TESTFILES_DIR} / "y86" / "y86_tb.v";
  auto reference_ift_tb = boost::filesystem::path{TESTFILES_DIR} / "y86" / "y86_ift_tb.v";
  auto reference_input = boost::filesystem::path{TESTFILES_DIR} / "y86" / "y86.txt";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  REQUIRE(created);
  auto exported_design = temp_dir / "y86TestIFT.v";

  auto design = ducode::Design::parse_json(json_file.string());
  design.write_verilog(exported_design, params);

  REQUIRE(boost::filesystem::exists(exported_design));

  auto data = ducode::simulate_design(exported_design, reference_ift_tb, reference_input);

  REQUIRE(boost::filesystem::exists(vcd_reference_file));

  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_reference_file.string());

  for (const ducode::Module& module: design.get_modules()) {
    module.write_graphviz(module.get_name() + ".dot");
  }

  // input output simulation should be enough for such a complex design.
  const auto& outer_scope = data->get_scope("main");
  const auto& outer_scope_reference = vcd_reference->get_scope("main");
  for (auto* scope_reference_signal: outer_scope_reference.signals) {
    auto signal_it = std::ranges::find_if(outer_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    spdlog::info(scope_reference_signal->reference);
    CHECK(signal_it != outer_scope.signals.end());
    CHECK(vcd_reference->get_signal_values(scope_reference_signal->hash) == data->get_signal_values((*signal_it)->hash));
  }

  bool contains_x = false;
  const auto& signals = data->get_signals();
  for (const auto& module: design.get_modules()) {
    for (const auto& port: module.get_ports()) {
      auto signal_it = std::ranges::find_if(signals, [&](const VCDSignal& signal) { return signal.reference == (port.m_name + ift_tag); });
      REQUIRE(signal_it != signals.end());
      auto signal_values = data->get_signal_values(signal_it->hash);
      for (const auto& signal_value: signal_values) {
        for (const auto& tag_bit: signal_value.value.get_value_vector()) {
          if (tag_bit == VCDBit::VCD_X) {
            contains_x = true;
            spdlog::debug("{} contains X", signal_it->reference);
            break;
          }
        }
      }
    }
  }
  CHECK_FALSE(contains_x);
}

TEST_CASE("export eth_fifo with ift", "[ift_test]") {
  nlohmann::json params;
  params["ift"] = true;

  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "eth_fifo" / "eth_fifo.json";
  auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / "eth_fifo" / "eth_fifo.vcd";
  auto reference_ift_tb = boost::filesystem::path{TESTFILES_DIR} / "eth_fifo" / "eth_fifo_ift_tb.v";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  REQUIRE(created);
  REQUIRE(boost::filesystem::exists(vcd_reference_file));

  auto design = ducode::Design::parse_json(json_file);
  auto exported_design = temp_dir / "exported_design.v";
  design.write_verilog(exported_design, params);
  REQUIRE(boost::filesystem::exists(exported_design));

  auto data = ducode::simulate_design(exported_design, reference_ift_tb);

  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_reference_file.string());

  const auto& outer_scope = data->get_scope("eth_fifo_tb");
  const auto& outer_scope_reference = vcd_reference->get_scope("eth_fifo_tb");
  for (auto* scope_reference_signal: outer_scope_reference.signals) {
    auto signal_it = std::ranges::find_if(outer_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    spdlog::info(scope_reference_signal->reference);
    REQUIRE(signal_it != outer_scope.signals.end());
    if (scope_reference_signal->type != VCDVarType::VCD_VAR_PARAMETER) {
      CHECK(vcd_reference->get_signal_values(scope_reference_signal->hash) == data->get_signal_values((*signal_it)->hash));
    }
  }

  bool contains_x = false;
  const auto& signals = data->get_signals();
  for (const auto& module: design.get_modules()) {
    for (const auto& port: module.get_ports()) {
      auto signal_it = std::ranges::find_if(signals, [&](const VCDSignal& signal) { return signal.reference == (port.m_name + ift_tag); });
      REQUIRE(signal_it != signals.end());
      auto signal_values = data->get_signal_values(signal_it->hash);
      for (const auto& signal_value: signal_values) {
        for (const auto& tag_bit: signal_value.value.get_value_vector()) {
          if (tag_bit == VCDBit::VCD_X) {
            contains_x = true;
            spdlog::debug("{} contains X", signal_it->reference);
            break;
          }
        }
      }
    }
  }
  CHECK_FALSE(contains_x);
}

TEST_CASE("counter_ift", "[ift_test]") {
  nlohmann::json params;
  params["ift"] = true;

  auto json_file = std::filesystem::path(TESTFILES_DIR) / "counter" / "counter.json";
  auto design = ducode::Design::parse_json(json_file.string());

  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  REQUIRE(created);

  auto instrumented_verilog = temp_dir / "iftverilog.v";
  design.write_verilog(instrumented_verilog, params);

  CHECK(boost::filesystem::exists(instrumented_verilog));

  auto design_file = std::filesystem::path(TESTFILES_DIR) / "counter" / "counter.v";
  auto testbench_file = std::filesystem::path(TESTFILES_DIR) / "counter" / "counter_tb.v";
  auto vcd_data = ducode::simulate_design(design_file, testbench_file);

  auto vcd_reference_file = std::filesystem::path(TESTFILES_DIR) / "counter" / "counter.vcd";
  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_reference_file.string());
  CHECK(ducode::no_repetitions_compare(*vcd_reference, *vcd_data));

  auto exported_testbench = temp_dir / "iftVerilog_tb.v";

  const ducode::Testbench testbench(design, vcd_data);
  testbench.write_verilog(exported_testbench, params);

  CHECK(boost::filesystem::exists(exported_testbench));

  auto vcd_data_ift = ducode::simulate_design(instrumented_verilog, exported_testbench);

  bool contains_x = false;
  const auto& signals = vcd_data_ift->get_signals();
  for (const auto& module: design.get_modules()) {
    for (const auto& port: module.get_ports()) {
      auto signal_it = std::ranges::find_if(signals, [&](const VCDSignal& signal) { return signal.reference == (port.m_name + ift_tag); });
      REQUIRE(signal_it != signals.end());
      auto signal_values = vcd_data_ift->get_signal_values(signal_it->hash);
      for (const auto& signal_value: signal_values) {
        for (const auto& tag_bit: signal_value.value.get_value_vector()) {
          if (tag_bit == VCDBit::VCD_X) {
            contains_x = true;
            spdlog::debug("{} contains X", signal_it->reference);
            break;
          }
        }
      }
    }
  }
  CHECK_FALSE(contains_x);
}