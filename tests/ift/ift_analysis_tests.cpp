/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/generate_tags.hpp>
#include <ducode/ift_analysis.hpp>
#include <ducode/instantiation_graph.hpp>
#include <ducode/sim.hpp>
#include <ducode/testbench.hpp>
#include <ducode/utility/iverilog_wrapper.hpp>

#include <boost/filesystem.hpp>
#include <catch2/catch_all.hpp>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <vcd-parser/VCDComparisons.hpp>

#include <chrono>
#include <iostream>
#include <string>


TEST_CASE("ift_analysis_test_multiple_edges_on_output", "[ift_analysis_test]") {
  nlohmann::json params;

  std::pair<std::string, std::string> designs = {"analysis_test", "clk"};

  nlohmann::json result_data_full;
  params["ift"] = true;

  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / designs.first / (designs.first + "_tb.v");
  auto json_file = std::filesystem::path(TESTFILES_DIR) / designs.first / (designs.first + ".json");
  auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / designs.first / (designs.first + ".vcd");

  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  auto exported_verilog = temp_dir / ("ift" + designs.first + ".v");
  auto exported_testbench = temp_dir / ("ift" + designs.first + "_tb.v");
  auto exported_tags = temp_dir / "vcd_input.txt";

  ducode::Design design = ducode::Design::parse_json(json_file);

  VCDFileParser vcd_obj;
  spdlog::info("Loading Original Simulation Data");
  if (!boost::filesystem::exists(vcd_reference_file_path)) {
    throw std::runtime_error("File does not exist!!");
  }
  auto vcd_data = vcd_obj.parse_file(vcd_reference_file_path.string());

  //export the design with IFT
  design.write_verilog(exported_verilog, params);

  //get top module
  auto tb_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });

  //create tags with full resolution
  auto tag_map = generate_tags_full_resolution(vcd_data->get_timestamps().back(), static_cast<uint32_t>(get_stepsize(vcd_data, designs.second)), tb_module->get_ports());

  ducode::DesignInstance instance = ducode::do_simulation(design, tb_module, vcd_data, exported_verilog, exported_testbench, exported_tags, tag_map, params);
  instance.write_graphviz("test.dot");
  //    auto stop_ift_simulation = std::chrono::high_resolution_clock::now();

  ///fill json object with relevant information about design -> then add it to result data json
  nlohmann::json ift_res = do_analysis(instance, design);
  int number_of_injected_tags = 0;
  for (auto& tag_vector: tag_map) {
    for (auto& tag: tag_vector.second) {
      number_of_injected_tags++;
    }
  }
  ift_res["number_of_injected_tags"] = number_of_injected_tags;

  CHECK(ift_res["number_of_injected_tags"] == 9);
  CHECK(ift_res["output_tag_count"]["analysis_test_c"] == 3);

  CHECK(ift_res["output_tag_count"]["analysis_test_d"] == 6);

  CHECK(ift_res["output_tag_count"]["total_tag_count"] == 9);
}


TEST_CASE("ift_analysis_test", "[ift_analysis_test]") {
  nlohmann::json params;

  std::pair<std::string, std::string> designs = {"analysis_test", "clk"};

  nlohmann::json result_data_full;
  params["ift"] = true;

  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / designs.first / (designs.first + "_tb.v");
  auto json_file = std::filesystem::path(TESTFILES_DIR) / designs.first / (designs.first + ".json");
  auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / designs.first / (designs.first + ".vcd");

  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  auto exported_verilog = temp_dir / ("ift" + designs.first + ".v");
  auto exported_testbench = temp_dir / ("ift" + designs.first + "_tb.v");
  auto exported_tags = temp_dir / "vcd_input.txt";

  ducode::Design design = ducode::Design::parse_json(json_file);

  VCDFileParser vcd_obj;
  spdlog::info("Loading Original Simulation Data");
  if (!boost::filesystem::exists(vcd_reference_file_path)) {
    throw std::runtime_error("File does not exist!!");
  }
  auto vcd_data = vcd_obj.parse_file(vcd_reference_file_path.string());

  //export the design with IFT
  design.write_verilog(exported_verilog, params);

  //get top module
  auto tb_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });

  //create tags with full resolution
  auto tag_map = generate_tags_full_resolution(vcd_data->get_timestamps().back(), static_cast<uint32_t>(get_stepsize(vcd_data, designs.second)), tb_module->get_ports());

  ducode::DesignInstance instance = ducode::do_simulation(design, tb_module, vcd_data, exported_verilog, exported_testbench, exported_tags, tag_map, params);
  instance.write_graphviz("test.dot");
  //    auto stop_ift_simulation = std::chrono::high_resolution_clock::now();

  ///fill json object with relevant information about design -> then add it to result data json
  nlohmann::json ift_res = do_analysis(instance, design);
  int number_of_injected_tags = 0;
  for (auto& tag_vector: tag_map) {
    for (auto& tag: tag_vector.second) {
      number_of_injected_tags++;
    }
  }
  ift_res["number_of_injected_tags"] = number_of_injected_tags;

  CHECK(ift_res["number_of_injected_tags"] == 9);
  CHECK(ift_res["output_tag_count"]["analysis_test_c"] == 3);

  CHECK(ift_res["output_tag_count"]["analysis_test_d"] == 6);

  CHECK(ift_res["output_tag_count"]["total_tag_count"] == 9);
}
