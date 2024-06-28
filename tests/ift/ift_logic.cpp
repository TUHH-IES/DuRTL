/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/instantiation_graph.hpp>
#include <ducode/utility/iverilog_wrapper.hpp>

#include <boost/filesystem.hpp>
#include <catch2/catch_all.hpp>
#include <spdlog/spdlog.h>

#include <string>

auto logic_test_body(const std::string& file_path) {
  nlohmann::json params;
  params["ift"] = true;

  auto json_file = boost::filesystem::path{TESTFILES_DIR} / (file_path + ".json");
  auto reference_tb = boost::filesystem::path{TESTFILES_DIR} / (file_path + "_tb.v");
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);
  auto verilog_export_file = temp_dir / "ift_design.v";

  auto design = ducode::Design::parse_json(json_file.string());

  spdlog::debug("temp_dir: {}", temp_dir.string());

  design.write_verilog(verilog_export_file, params);

  REQUIRE(boost::filesystem::exists(verilog_export_file));

  auto data = ducode::simulate_design(verilog_export_file, reference_tb);
  auto instance = ducode::DesignInstance::create_instance(design);
  instance.add_vcd_data(data);

  return instance;
}

TEST_CASE("mux_logic", "[ift_logic]") {
  auto instance = logic_test_body("mux/muxGate");

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

TEST_CASE("and_logic", "[ift_logic]") {
  auto instance = logic_test_body("and/and_x_test");

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

TEST_CASE("or_logic", "[ift_logic]") {
  auto instance = logic_test_body("or/or_x_test");

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