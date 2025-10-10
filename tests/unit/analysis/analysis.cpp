/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/instantiation_graph.hpp>
#include <ducode/instantiation_graph_traits.hpp>
#include <ducode/signals_data_manager.hpp>
#include <ducode/tag_generator.hpp>
#include <ducode/testbench.hpp>
#include <ducode/utility/iverilog_wrapper.hpp>
#include <ducode/utility/simulation.hpp>

#include "ducode/utility/VCD_utility.hpp"
#include "ducode/utility/types.hpp"
#include <boost/filesystem/operations.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/detail/adjacency_list.hpp>
#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>
#include <gsl/narrow>
#include <nlohmann/json_fwd.hpp>
#include <spdlog/spdlog.h>
#include <vcd-parser/VCDFile.hpp>
#include <vcd-parser/VCDTypes.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <ranges>
#include <string>
#include <utility>
#include <vector>


namespace {
inline ducode::DesignInstance create_spi_master_instance() {
  nlohmann::json params;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;

  const std::string design_file = "SPI_Master";
  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / design_file / (design_file + "_tb.v");
  auto json_file = std::filesystem::path(TESTFILES_DIR) / design_file / (design_file + ".json");
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  REQUIRE(created);

  auto exported_verilog = temp_dir / ("ift" + design_file + ".v");
  auto exported_testbench = temp_dir / ("ift" + design_file + "_tb.v");

  auto design = ducode::Design::parse_json(json_file);
  params["ift"] = false;
  design.write_verilog(exported_verilog, params);

  auto vcd_data = ducode::simulate_design(exported_verilog, testbench_file);

  const auto stepsize = gsl::narrow<uint32_t>(ducode::get_stepsize(vcd_data, "clk"));

  auto top_module = design.get_top_module();

  params["ift"] = true;
  design.write_verilog(exported_verilog, params);

  auto timesteps_per_simulation_run = tag_size / top_module.get_input_names().size();
  timesteps_per_simulation_run = std::max<size_t>(timesteps_per_simulation_run, 1);
  auto number_simulation_runs = ducode::get_number_of_simulation_runs(stepsize, timesteps_per_simulation_run, vcd_data);
  params["timesteps_per_simulation_run"] = timesteps_per_simulation_run;

  spdlog::info("Running {} simulations", number_simulation_runs);
  //get top module

  //exporting testbench with IFT
  const VCDScope* root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));
  testbench.set_tag_generator(std::make_unique<ducode::FullResolutionTagGenerator>(top_module.get_input_ports(), vcd_data));
  testbench.write_verilog(exported_testbench, params);

  auto instance = ducode::DesignInstance::create_instance(design);
  ducode::VCDSignalsDataManager test_value_map;

  std::vector<boost::filesystem::path> exported_tags_files;
  for (uint32_t i = 0; i < number_simulation_runs; ++i) {
    boost::filesystem::path tag_file = temp_dir / ("vcd_input_" + std::to_string(i) + ".txt");
    exported_tags_files.push_back(tag_file);
  }


  std::vector<VCDTime> tags_times;
  for (auto time = 0u; time <= gsl::narrow<std::size_t>(vcd_data->get_timestamps().back()); time += stepsize) {
    tags_times.emplace_back(time);
  }

  testbench.write_verilog(exported_testbench, params);
  std::vector<ExportedTagsFile> exported_tags_vec;

  /// here add loop which runs the simulation again for all created tag_map files.
  for (uint32_t i = 0; i < number_simulation_runs; ++i) {
    // For the ith batch of timestamps
    uint32_t items_per_batch = timesteps_per_simulation_run;
    uint32_t start_idx = i * items_per_batch;
    uint32_t end_idx = std::min(((i + 1) * items_per_batch), gsl::narrow<uint32_t>(tags_times.size()));

    std::vector<VCDTime> batch(tags_times.begin() + start_idx,
                               tags_times.begin() + end_idx);

    exported_tags_vec.emplace_back(batch, exported_tags_files[i]);
  }
  testbench.write_tags(exported_tags_vec, params);

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;
  /// IFT simulation
  spdlog::info("Simulating Exported Design + Testbench");
  for (const auto& tags: exported_tags_files) {
    auto data = ducode::simulate_design(exported_verilog, exported_testbench, tags);
    spdlog::info("End of Simulation");
    vcd_data_vector.emplace_back(data);
  }
  std::vector<const VCDScope*> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto& vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager sim_data(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(sim_data));


  return instance;
}

inline ducode::DesignInstance create_design_instance(const std::string& design_file) {
  nlohmann::json params;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;

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
  auto top_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });

  const int stepsize = 8;

  auto timesteps_per_simulation_run = tag_size / top_module->get_input_names().size();
  timesteps_per_simulation_run = std::max<size_t>(timesteps_per_simulation_run, 1);

  params["timesteps_per_simulation_run"] = timesteps_per_simulation_run;

  //exporting testbench with IFT
  const VCDScope* root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));
  testbench.set_tag_generator(std::make_unique<ducode::FullResolutionTagGenerator>(top_module->get_input_ports(), vcd_data));
  testbench.write_verilog(exported_testbench, params);
  const std::vector<VCDTime> timesteps = {0, 8, 16, 24};
  std::vector<ExportedTagsFile> exported_tags_ = {ExportedTagsFile{.time_points = timesteps, .exported_tags_file_path = exported_tags}};

  testbench.write_tags(exported_tags_, params);

  /// IFT simulation
  auto data = ducode::simulate_design(exported_verilog, exported_testbench, exported_tags);

  auto instance = ducode::DesignInstance::create_instance(design);
  const VCDScope* root_scope_test = &ducode::find_root_scope(data, design);
  ducode::VCDSignalsDataManager test_value_map(data, root_scope_test);
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  return instance;
}
}// namespace

TEST_CASE("most_toggled_path", "[analysis]") {
  auto instance = create_spi_master_instance();

  auto most_t_path = instance.most_toggled_path();
  auto previous_step = most_t_path.front();
  for (const auto& step: most_t_path | std::ranges::views::drop(1)) {
    spdlog::info("{} - {}; toggles: {}\n", instance.m_graph[previous_step].name, instance.m_graph[step].name, instance.get_signal_values(boost::edge(previous_step, step, instance.m_graph).first).m_timed_signal_values.size());
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
    spdlog::info("{} - {}; toggles: {}\n", instance.m_graph[previous_step].name, instance.m_graph[step].name, instance.get_signal_values(boost::edge(previous_step, step, instance.m_graph).first).m_timed_signal_values.size());
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

TEST_CASE("tag_path_from_input", "[analysis][.]") {
  auto instance = create_spi_master_instance();
  std::string input_str = "cpha";
  const ducode::SignalTime time_step = 0;
  auto path = instance.tag_path_from_input(input_str, time_step);

  for (const auto& step: path) {
    spdlog::info("{}", instance.m_graph[step].name);
  }

  //  CHECK(path.size() == 6);
  WARN("The correctness of the results need to be verified manually!");
}
