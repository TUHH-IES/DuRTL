/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/instantiation_graph.hpp>
#include <ducode/utility/iverilog_wrapper.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <catch2/catch_all.hpp>
#include <vcd-parser/VCDFileParser.hpp>

#include <algorithm>
#include <string>
#include <utility>

std::pair<std::shared_ptr<VCDFile>, std::shared_ptr<VCDFile>> simulate_instantiation_graph(const std::string& design_file);

TEST_CASE("instantiate_basic", "[instantiation_graph]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "hierarchy" / "hierarchy.json";
  auto design = ducode::Design::parse_json(json_file);
  auto instance = ducode::DesignInstance::create_instance(design);

  instance.write_graphviz("graph.dot");

  CHECK(boost::num_vertices(instance.m_graph) == 29);
  CHECK(boost::num_edges(instance.m_graph) == 35);
}

TEST_CASE("simulate spi_master flattened", "[instantiation_graph]") {
  const std::string design_file = "SPI_Master";
  auto [vcd_data, _] = simulate_instantiation_graph(design_file);
  auto vcd_file = boost::filesystem::path{TESTFILES_DIR} / design_file / (design_file + "_exported.vcd");
  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_file.string());

  auto json_file = boost::filesystem::path{TESTFILES_DIR} / design_file / (design_file + ".json");
  auto design = ducode::Design::parse_json(json_file);

  const auto& outer_scope = vcd_data->get_scope("uut");
  const auto& outer_scope_reference = vcd_reference->get_scope("uut");
  for (const auto& port: design.get_module("SPI_Master").get_ports()) {

    auto lambda = [&](const auto& signal) { return signal->reference == port.m_name; };
    auto hash_iter = std::ranges::find_if(outer_scope.signals, lambda);
    auto hash_reference_iter = std::ranges::find_if(outer_scope_reference.signals, lambda);

    const auto& signal_values = vcd_data->get_signal_values((*hash_iter)->hash);
    const auto& signal_values_reference = vcd_reference->get_signal_values((*hash_reference_iter)->hash);
    CHECK(signal_values == signal_values_reference);
  }
}

TEST_CASE("simulate hierarchy flattened", "[instantiation_graph]") {
  auto [data, vcd_reference] = simulate_instantiation_graph("hierarchy");

  const auto& inner_scope = data->get_scope("hierarchy_tb");
  const auto& inner_scope_reference = vcd_reference->get_scope("hierarchy_tb");
  for (auto* scope_reference_signal: inner_scope_reference.signals) {
    auto signal_it = std::ranges::find_if(inner_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    CHECK(signal_it != inner_scope.signals.end());
    CHECK(vcd_reference->get_signal_values(scope_reference_signal->hash) == data->get_signal_values((*signal_it)->hash));
  }
}

TEST_CASE("add VCD data to instantian_graph", "[instantiation_graph]") {
  const std::string design_file = "hierarchy";
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / design_file / (design_file + ".json");
  auto vcd_file = boost::filesystem::path{TESTFILES_DIR} / design_file / (design_file + ".vcd");
  auto reference_tb = boost::filesystem::path{TESTFILES_DIR} / design_file / (design_file + "_tb.v");

  auto design = ducode::Design::parse_json(json_file);
  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_file.string());
  auto instance = ducode::DesignInstance::create_instance(design);
  instance.add_vcd_data(vcd_reference);

  boost::graph_traits<ducode::DesignInstance::instantiation_graph>::edge_iterator ei;
  boost::graph_traits<ducode::DesignInstance::instantiation_graph>::edge_iterator ei_end;
  for (boost::tie(ei, ei_end) = boost::edges(instance.m_graph); ei != ei_end; ++ei) {
    CHECK(instance.m_graph[*ei].net_ptr != nullptr);
  }
}

std::pair<std::shared_ptr<VCDFile>, std::shared_ptr<VCDFile>> simulate_instantiation_graph(const std::string& design_file) {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / design_file / (design_file + ".json");
  auto vcd_file = boost::filesystem::path{TESTFILES_DIR} / design_file / (design_file + ".vcd");
  auto reference_tb = boost::filesystem::path{TESTFILES_DIR} / design_file / (design_file + "_tb.v");

  auto design = ducode::Design::parse_json(json_file);
  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_file.string());
  auto instance = ducode::DesignInstance::create_instance(design);

  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  REQUIRE(created);

  auto exported_design = temp_dir / "flatten.v";
  instance.write_verilog(exported_design);
  REQUIRE(boost::filesystem::exists(exported_design));

  auto vcd_data = ducode::simulate_design(exported_design, reference_tb);
  REQUIRE(vcd_data != nullptr);

  return std::make_pair(vcd_data, vcd_reference);
}