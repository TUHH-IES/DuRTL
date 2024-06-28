/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/sim.hpp>

#include <boost/filesystem.hpp>
#include <catch2/catch_all.hpp>
#include <nlohmann/json.hpp>

double get_stepsize(auto& vcd_data, const std::string& clk_name) {

  std::deque<VCDTimedValue> clk_values;

  for (const VCDScope* scope: vcd_data->get_scopes().at(0).children) {
    for (const VCDSignal* signal: scope->signals) {
      if (signal->reference == clk_name) {
        clk_values = vcd_data->get_signal_values(signal->hash);
        break;
      }
    }
    if (!clk_values.empty()) {
      break;
    }
  }

  return clk_values[2].time;
}

TEST_CASE("complete_ift_y86", "[long][.]") {
  nlohmann::json params;
  params["ift"] = true;
  const std::string design_name = "y86";
  const std::string signal_name = "clk";

  auto verilog_file = boost::filesystem::path(TESTFILES_DIR) / design_name / (design_name + ".v");
  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / design_name / (design_name + "_tb.v");
  auto json_file = boost::filesystem::path(TESTFILES_DIR) / design_name / (design_name + ".json");
  auto reference_input = boost::filesystem::path{TESTFILES_DIR} / "y86" / "test_0.txt";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  auto design = ducode::Design::parse_json(json_file);

  auto vcd_data = ducode::simulate_design(verilog_file, testbench_file, reference_input);

  auto exported_verilog = temp_dir / "ift_design.v";
  auto exported_testbench = temp_dir / "ift_testbench.v";
  auto exported_tags = temp_dir / "vcd_input.txt";

  design.write_verilog(exported_verilog, params);

  auto top_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });
  REQUIRE(top_module != design.get_modules().end());

  auto tag_map = generate_tags_full_resolution(vcd_data->get_timestamps().back(), static_cast<uint32_t>(get_stepsize(vcd_data, signal_name)), top_module->get_ports());

  auto instance = ducode::do_simulation(design, top_module, vcd_data, exported_verilog, exported_testbench, exported_tags, tag_map, params);

  bool contains_x = false;

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

TEST_CASE("complete_ift_aes128", "[long][.]") {
  nlohmann::json params;
  params["ift"] = true;
  const std::vector<std::pair<std::string, std::string>> designs = {{"aes_128", "clk"}};

  const std::string design_name = "aes_128";
  const std::string signal_name = "clk";

  auto verilog_file = boost::filesystem::path(TESTFILES_DIR) / design_name / (design_name + ".v");
  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / design_name / (design_name + "_tb.v");
  auto json_file = boost::filesystem::path(TESTFILES_DIR) / design_name / (design_name + ".json");
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  auto design = ducode::Design::parse_json(json_file);

  auto exported_verilog = temp_dir / "ift_design.v";
  auto exported_testbench = temp_dir / "ift_testbench.v";
  auto exported_tags = temp_dir / "vcd_input.txt";

  design.write_verilog(exported_verilog, params);

  auto top_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });
  REQUIRE(top_module != design.get_modules().end());

  auto vcd_data = ducode::simulate_design(verilog_file, testbench_file);

  auto tag_map = generate_tags_full_resolution(vcd_data->get_timestamps().back(), static_cast<uint32_t>(get_stepsize(vcd_data, designs[0].second)), top_module->get_ports());

  auto instance = ducode::do_simulation(design, top_module, vcd_data, exported_verilog, exported_testbench, exported_tags, tag_map, params);

  bool contains_x = false;

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