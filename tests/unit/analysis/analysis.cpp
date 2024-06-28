/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/generate_tags.hpp>
#include <ducode/instantiation_graph.hpp>
#include <ducode/testbench.hpp>
#include <ducode/utility/iverilog_wrapper.hpp>

#include <boost/filesystem.hpp>
#include <catch2/catch_all.hpp>
#include <spdlog/spdlog.h>
#include <vcd-parser/VCDComparisons.hpp>

#include <string>

inline ducode::DesignInstance create_spi_master_instance() {
  nlohmann::json params;
  params["apprx"] = false;

  const std::string design_file = "SPI_Master";
  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / design_file / (design_file + "_tb.v");
  auto json_file = std::filesystem::path(TESTFILES_DIR) / design_file / (design_file + ".json");
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  REQUIRE(created);

  auto exported_verilog = temp_dir / ("ift" + design_file + ".v");
  auto exported_testbench = temp_dir / ("ift" + design_file + "_tb.v");
  auto exported_tags = temp_dir / "vcd_input.txt";

  auto design = ducode::Design::parse_json(json_file);
  params["ift"] = false;
  design.write_verilog(exported_verilog, params);

  auto vcd_data = ducode::simulate_design(exported_verilog, testbench_file);

  params["ift"] = true;
  design.write_verilog(exported_verilog, params);

  //get top module
  auto tb_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });

  const int stepsize = 1;
  auto tag_map = generate_tags_full_resolution(vcd_data->get_timestamps().back(), stepsize, tb_module->get_ports());
  auto tag_map_split = tag_map_splitting(tb_module, tag_map);

  /// only using s single simulation run and corresponding tagging file (tag_map[0]) for testbench generation
  auto encoded_tags = encode_tags(tag_map_split[0]);
  export_tags(tb_module, exported_tags.string(), encoded_tags);

  //exporting testbench with IFT
  const ducode::Testbench testbench(design, vcd_data);
  testbench.write_verilog(exported_testbench, exported_tags, params);

  /// IFT simulation
  auto data = ducode::simulate_design(exported_verilog, exported_testbench, exported_tags);

  auto instance = ducode::DesignInstance::create_instance(design);
  instance.add_vcd_data(data);
  instance.write_graphviz("graph.dot");

  return instance;
}

inline ducode::DesignInstance create_design_instance(const std::string& design_file) {
  nlohmann::json params;
  params["apprx"] = false;

  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / design_file / (design_file + "_tb.v");
  auto json_file = std::filesystem::path(TESTFILES_DIR) / design_file / (design_file + ".json");
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  REQUIRE(created);

  auto exported_verilog = temp_dir / ("ift" + design_file + ".v");
  auto exported_testbench = temp_dir / ("ift" + design_file + "_tb.v");
  auto exported_tags = temp_dir / "vcd_input.txt";

  auto design = ducode::Design::parse_json(json_file);
  params["ift"] = false;
  design.write_verilog(exported_verilog, params);

  auto vcd_data = ducode::simulate_design(exported_verilog, testbench_file);

  params["ift"] = true;
  design.write_verilog(exported_verilog, params);

  //get top module
  auto tb_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });

  const int stepsize = 1;
  auto tag_map = generate_tags_full_resolution(vcd_data->get_timestamps().back(), stepsize, tb_module->get_ports());
  auto tag_map_split = tag_map_splitting(tb_module, tag_map);

  /// only using s single simulation run and corresponding tagging file (tag_map[0]) for testbench generation
  auto encoded_tags = encode_tags(tag_map_split[0]);
  export_tags(tb_module, exported_tags.string(), encoded_tags);

  //exporting testbench with IFT
  const ducode::Testbench testbench(design, vcd_data);
  testbench.write_verilog(exported_testbench, exported_tags, params);

  /// IFT simulation
  auto data = ducode::simulate_design(exported_verilog, exported_testbench, exported_tags);

  auto instance = ducode::DesignInstance::create_instance(design);
  instance.add_vcd_data(data);
  instance.write_graphviz("graph.dot");

  return instance;
}

TEST_CASE("most_toggled_path", "[analysis]") {
  auto instance = create_spi_master_instance();

  auto most_t_path = instance.most_toggled_path();
  auto previous_step = most_t_path.front();
  for (const auto& step: most_t_path | std::ranges::views::drop(1)) {
    spdlog::info("{} - {}; toggles: {}\n", instance.m_graph[previous_step].name, instance.m_graph[step].name, instance.get_signal_values(boost::edge(previous_step, step, instance.m_graph).first)->size());
    previous_step = step;
  }

  //  CHECK(instance.m_graph.m_vertices[most_t_path[0]].m_property.name == "$procmux$917");
  //  CHECK(instance.m_graph.m_vertices[most_t_path[1]].m_property.name == "$auto$ff.cc:266:slice$1117");
  //  CHECK(most_t_path.size() == 14);
  WARN("The correctness of the results need to be verified manually!");
}

TEST_CASE("most_toggled_path_clear_result", "[analysis]") {
  auto instance = create_design_instance("hierarchy");

  auto most_t_path = instance.most_toggled_path();
  auto previous_step = most_t_path.front();
  for (const auto& step: most_t_path | std::ranges::views::drop(1)) {
    spdlog::info("{} - {}; toggles: {}\n", instance.m_graph[previous_step].name, instance.m_graph[step].name, instance.get_signal_values(boost::edge(previous_step, step, instance.m_graph).first)->size());
    previous_step = step;
  }

  CHECK(instance.m_graph.m_vertices[most_t_path[0]].m_property.name == "hierarchy_b");
  CHECK(instance.m_graph.m_vertices[most_t_path[1]].m_property.name == "M_b");
  CHECK(instance.m_graph.m_vertices[most_t_path[most_t_path.size() - 1]].m_property.name == "hierarchy_c");
  CHECK((most_t_path.size() == 14 || most_t_path.size() == 17));
  //  WARN("The correctness of the results need to be verified manually!");
}

TEST_CASE("least_tagged_path", "[analysis]") {
  auto instance = create_spi_master_instance();

  auto path = instance.least_tagged_path();

  for (const auto& step: path) {
    spdlog::info("{}", instance.m_graph[step].name);
  }

  //  CHECK(path.size() == 6);
  WARN("The correctness of the results need to be verified manually!");
}

TEST_CASE("most_tagged_path", "[analysis]") {
  auto instance = create_spi_master_instance();

  auto path = instance.most_tagged_path();

  for (const auto& step: path) {
    spdlog::info("{}", instance.m_graph[step].name);
  }

  //  CHECK(path.size() == 6);
  WARN("The correctness of the results need to be verified manually!");
}

TEST_CASE("x_tag_path", "[analysis]") {
  auto instance = create_spi_master_instance();

  auto path = instance.x_tag_path();

  for (const auto& step: path) {
    spdlog::info("{}", instance.m_graph[step].name);
  }

  //  CHECK(path.size() == 6);
  WARN("The correctness of the results need to be verified manually!");
}

TEST_CASE("tag_path_from_input", "[analysis]") {
  auto instance = create_spi_master_instance();
  std::string input_str = "cpha";
  const double time_step = 0;
  auto path = instance.tag_path_from_input(input_str, time_step);

  for (const auto& step: path) {
    spdlog::info("{}", instance.m_graph[step].name);
  }

  //  CHECK(path.size() == 6);
  WARN("The correctness of the results need to be verified manually!");
}