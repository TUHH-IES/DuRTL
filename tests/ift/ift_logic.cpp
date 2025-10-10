/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/instantiation_graph.hpp>
#include <ducode/instantiation_graph_traits.hpp>
#include <ducode/signals_data_manager.hpp>
#include <ducode/utility/VCD_utility.hpp>
#include <ducode/utility/iverilog_wrapper.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/detail/adjacency_list.hpp>
#include <catch2/catch_test_macros.hpp>
#include <fmt/core.h>
#include <nlohmann/json_fwd.hpp>
#include <spdlog/spdlog.h>
#include <vcd-parser/VCDTypes.hpp>

#include <cassert>
#include <cstdint>
#include <memory>
#include <string>


namespace {
auto logic_test_body(const std::string& file_path) {
  nlohmann::json params;
  params["ift"] = true;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;

  auto json_file = boost::filesystem::path{TESTFILES_DIR} / (file_path + ".json");
  auto reference_tb = boost::filesystem::path{TESTFILES_DIR} / (file_path + "_tb.v");
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);
  auto verilog_export_file = temp_dir / "ift_design.v";

  auto design = ducode::Design::parse_json(json_file);

  spdlog::debug("temp_dir: {}", temp_dir.string());

  design.write_verilog(verilog_export_file, params);

  REQUIRE(boost::filesystem::exists(verilog_export_file));

  auto data = ducode::simulate_design(verilog_export_file, reference_tb);
  auto instance = ducode::DesignInstance::create_instance(design);
  const VCDScope* root_scope = &ducode::find_root_scope(data, design);
  ducode::VCDSignalsDataManager test_value_map(data, root_scope);
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));
  return instance;
}
}// anonymous namespace

TEST_CASE("verilog_export_x_test", "[ift_logic]") {
  const auto design_list = {"mux/muxGate",
                            "or/or_x_test",
                            "and/and_x_test"};

  for (const auto& design: design_list) {
    auto instance = logic_test_body(fmt::format("verilog_export_x_test/{}", design));

    bool contains_x = false;

    for (auto [edge_it, edge_end] = boost::edges(instance.m_graph); edge_it != edge_end; ++edge_it) {
      if (instance.m_graph[*edge_it].net_ptr != nullptr) {
        auto tag_values = instance.get_tag_values(*edge_it);
        for (const auto& tag_vector: tag_values) {
          for (const auto& tag_value: tag_vector.m_timed_signal_values) {
            for (const auto& tag_bit: tag_value.value.get_value_vector()) {
              if (tag_bit == ducode::SignalBit::BIT_X) {
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
}