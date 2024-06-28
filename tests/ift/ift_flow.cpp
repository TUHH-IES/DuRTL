/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/generate_tags.hpp>
#include <ducode/sim.hpp>
#include <ducode/testbench.hpp>
#include <ducode/utility/VCD_comparisons.hpp>
#include <ducode/utility/VCD_printers.hpp>
#include <ducode/utility/iverilog_wrapper.hpp>

#include <boost/filesystem.hpp>
#include <catch2/catch_all.hpp>
#include <spdlog/spdlog.h>
#include <vcd-parser/VCDComparisons.hpp>

#include <iostream>
#include <string>

TEST_CASE("hierarchy_ift_flow", "[ift_flow]") {
  nlohmann::json params;
  params["ift"] = true;
  params["apprx"] = false;

  auto verilog_file = boost::filesystem::path(TESTFILES_DIR) / "ift_flow" / "hierarchy.v";
  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / "ift_flow" / "hierarchy_ift_flow_tb.v";
  auto json_file = boost::filesystem::path(TESTFILES_DIR) / "ift_flow" / "hierarchy.json";
  auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / "ift_flow" / "hierarchy_ift";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  auto design = ducode::Design::parse_json(json_file);

  auto vcd_data = ducode::simulate_design(verilog_file, testbench_file);

  auto exported_verilog = temp_dir / "ift_design.v";
  auto exported_testbench = temp_dir / "ift_testbench.v";
  auto exported_tags = temp_dir / "vcd_input.txt";

  design.write_verilog(exported_verilog, params);

  auto top_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });
  REQUIRE(top_module != design.get_modules().end());

  const int stepsize = 8;
  auto tag_map = generate_tags_full_resolution(vcd_data->get_timestamps().back(), stepsize, top_module->get_ports());

  auto instance = ducode::do_simulation(design, top_module, vcd_data, exported_verilog, exported_testbench, exported_tags, tag_map, params);
  instance.write_graphviz("hierarchy.dot");
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

  auto find_edge = [&](std::string_view edge_name, std::vector<std::string>& hierarchy) {
    for (auto& edg: instance.m_graph.m_edges) {
      if (instance.m_graph.m_vertices[edg.m_target].m_property.name == edge_name && instance.m_graph.m_vertices[edg.m_target].m_property.hierarchy == hierarchy) {
        return edg;
      }
    }
    throw std::runtime_error("name not found!");
  };
  std::vector<std::string> hierarchy_vec = {"hierarchy", "m1", "s1"};
  std::vector<const VCDSignalValues*> values = instance.get_tag_values(find_edge("S_c", hierarchy_vec));
  CHECK(values[0][0][0].value.get_value_vector()[0] == VCDBit::VCD_1);
  CHECK(values[0][0][0].value.get_value_vector()[1] == VCDBit::VCD_0);

  CHECK(values[0][0][1].value.get_value_vector()[0] == VCDBit::VCD_1);
  CHECK(values[0][0][1].value.get_value_vector()[1] == VCDBit::VCD_1);
  CHECK(values[0][0][1].value.get_value_vector()[2] == VCDBit::VCD_0);
  CHECK(values[0][0][1].value.get_value_vector()[3] == VCDBit::VCD_0);

  hierarchy_vec = {"hierarchy", "m1", "s2"};
  values = instance.get_tag_values(find_edge("S_c", hierarchy_vec));
  CHECK(values[0][0][0].value.get_value_vector()[0] == VCDBit::VCD_1);
  CHECK(values[0][0][0].value.get_value_vector()[1] == VCDBit::VCD_0);

  CHECK(values[0][0][1].value.get_value_vector()[0] == VCDBit::VCD_1);
  CHECK(values[0][0][1].value.get_value_vector()[1] == VCDBit::VCD_1);
  CHECK(values[0][0][1].value.get_value_vector()[2] == VCDBit::VCD_0);
  CHECK(values[0][0][1].value.get_value_vector()[3] == VCDBit::VCD_0);

  hierarchy_vec = {"hierarchy"};
  values = instance.get_tag_values(find_edge("hierarchy_c", hierarchy_vec));
  CHECK(values[0][0][0].value.get_value_vector()[0] == VCDBit::VCD_1);
  CHECK(values[0][0][0].value.get_value_vector()[1] == VCDBit::VCD_0);

  CHECK(values[0][0][1].value.get_value_vector()[0] == VCDBit::VCD_1);
  CHECK(values[0][0][1].value.get_value_vector()[1] == VCDBit::VCD_1);
  CHECK(values[0][0][1].value.get_value_vector()[2] == VCDBit::VCD_0);
  CHECK(values[0][0][1].value.get_value_vector()[3] == VCDBit::VCD_0);

  CHECK(values[0][0][3].value.get_value_vector()[0] == VCDBit::VCD_1);
  CHECK(values[0][0][3].value.get_value_vector()[1] == VCDBit::VCD_1);
  CHECK(values[0][0][3].value.get_value_vector()[2] == VCDBit::VCD_0);
  CHECK(values[0][0][3].value.get_value_vector()[3] == VCDBit::VCD_0);
  CHECK(values[0][0][3].value.get_value_vector()[4] == VCDBit::VCD_0);
  CHECK(values[0][0][3].value.get_value_vector()[5] == VCDBit::VCD_0);
  CHECK(values[0][0][3].value.get_value_vector()[6] == VCDBit::VCD_0);
  CHECK(values[0][0][3].value.get_value_vector()[7] == VCDBit::VCD_0);
}

TEST_CASE("ift_flow_fft16", "[ift_flow]") {
  nlohmann::json params;
  params["ift"] = true;

  const int stepsize = 2000000;
  auto json_file = std::filesystem::path(TESTFILES_DIR) / "fft" / "fft16.json";
  auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / "fft" / "fft16.vcd";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  auto design = ducode::Design::parse_json(json_file);
  VCDFileParser vcd_obj;
  auto vcd_data = vcd_obj.parse_file(vcd_reference_file_path.string());

  auto exported_verilog = temp_dir / "ift_design.v";
  auto exported_testbench = temp_dir / "ift_testbench.v";
  auto exported_tags = temp_dir / "vcd_input.txt";

  auto tb_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });
  auto tag_map = generate_tags_full_resolution(vcd_data->get_timestamps().back(), stepsize, tb_module->get_ports());

  design.write_verilog(exported_verilog, params);
  auto tag_map_split = tag_map_splitting(tb_module, tag_map);
  /// only using s single simulation run and corresponding tagging file (tag_map[0]) for testbench generation
  auto encoded_tags = encode_tags(tag_map_split[0]);
  export_tags(tb_module, exported_tags.string(), encoded_tags);

  auto instance = ducode::do_simulation(design, tb_module, vcd_data, exported_verilog, exported_testbench, exported_tags, tag_map, params);

  instance.write_graphviz("graph.dot");

  bool contains_x = false;

  for (auto [edge_it, edge_end] = boost::edges(instance.m_graph); edge_it != edge_end; ++edge_it) {
    if (instance.m_graph[*edge_it].net_ptr != nullptr) {
      auto tag_values = instance.get_tag_values(*edge_it);
      for (const auto* tag_vector: tag_values) {
        for (const auto& tag_value: *tag_vector) {
          for (const auto& tag_bit: tag_value.value.get_value_vector()) {
            if (tag_bit == VCDBit::VCD_X) {
              spdlog::debug("{} contains x", instance.m_graph[*edge_it].net_ptr->get_name());
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