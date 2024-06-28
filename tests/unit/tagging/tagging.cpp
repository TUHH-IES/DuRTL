/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/generate_tags.hpp>
#include <ducode/testbench.hpp>
#include <ducode/utility/iverilog_wrapper.hpp>

#include <boost/filesystem.hpp>
#include <catch2/catch_all.hpp>
#include <spdlog/spdlog.h>
#include <vcd-parser/VCDComparisons.hpp>

#include <string>

TEST_CASE("tagging_inputs_multi_timestep", "[tagging_tests]") {
  nlohmann::json params;

  const std::vector<std::string> designs = {"and"};

  nlohmann::json result_data_full;

  params["ift"] = false;
  params["apprx"] = false;

  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / designs[0] / (designs[0] + "_tb.v");
  auto json_file = std::filesystem::path(TESTFILES_DIR) / designs[0] / (designs[0] + ".json");
  auto dot_file = std::filesystem::path(TESTFILES_DIR) / designs[0] / "graph.dot";

  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  auto exported_ift_verilog = temp_dir / ("ift" + designs[0] + ".v");
  auto exported_verilog = temp_dir / (designs[0] + ".v");
  auto exported_testbench = temp_dir / ("ift" + designs[0] + "_tb.v");
  auto exported_tags = temp_dir / "vcd_input.txt";

  auto design = ducode::Design::parse_json(json_file);
  design.write_verilog(exported_verilog, params);

  // original design Simulation
  spdlog::info("Simulating Original Design + Testbench");
  auto vcd_data = ducode::simulate_design(exported_verilog, testbench_file);
  spdlog::info("End of Simulation");

  params["ift"] = true;
  design.write_verilog(exported_ift_verilog, params);

  //get top module
  auto tb_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });

  const int stepsize = 1;
  //create tags with full resolution
  auto tag_map = generate_tags_full_resolution(vcd_data->get_timestamps().back(), stepsize, tb_module->get_ports());
  auto tag_map_split = tag_map_splitting(tb_module, tag_map);

  /// only using s single simulation run and corresponding tagging file (tag_map[0]) for testbench generation
  std::map<uint32_t, std::pair<uint32_t, ducode::Port>> encoded_tags = encode_tags(tag_map_split[0]);
  export_tags(tb_module, exported_tags.string(), encoded_tags);

  //exporting testbench with IFT
  const ducode::Testbench testbench(design, vcd_data);
  testbench.write_verilog(exported_testbench, exported_tags, params);

  /// IFT simulation tag_map[0]
  spdlog::info("Simulating Exported Design + Testbench");
  auto data = ducode::simulate_design(exported_ift_verilog, exported_testbench, exported_tags);
  spdlog::info("End of Simulation");

  // checking number of entries into the tag vectors
  CHECK(data->get_signal_values(std::ranges::find_if(data->get_signals(), [&](const auto& sig) { return (sig.reference == "a_t"); })->hash).size() == 5);
  CHECK(data->get_signal_values(std::ranges::find_if(data->get_signals(), [&](const auto& sig) { return (sig.reference == "b_t"); })->hash).size() == 5);
  CHECK(data->get_signal_values(std::ranges::find_if(data->get_signals(), [&](const auto& sig) { return (sig.reference == "c_t"); })->hash).size() == 5);

  //  CHECK(instance.m_graph.m_edges.size() == 3);
  //  CHECK(instance.m_graph.m_edges.front().get_property().name == "a");
  //  CHECK(instance.m_graph.m_edges.front().get_property().tags.size() == 5);
  //  CHECK(instance.m_graph.m_edges.front().get_property().values.size() == 4);
}
