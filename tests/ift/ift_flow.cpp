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
#include <ducode/utility/subrange.hpp>
#include <ducode/utility/types.hpp>

#include "ducode/utility/VCD_utility.hpp"
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/detail/adjacency_list.hpp>
#include <catch2/catch_test_macros.hpp>
#include <gsl/narrow>
#include <nlohmann/json_fwd.hpp>
#include <spdlog/spdlog.h>
#include <vcd-parser/VCDFile.hpp>
#include <vcd-parser/VCDFileParser.hpp>
#include <vcd-parser/VCDTypes.hpp>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

TEST_CASE("hierarchy_ift_flow_testbench", "[ift_flow]") {
  nlohmann::json params;
  params["ift"] = true;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;

  auto verilog_file = boost::filesystem::path(TESTFILES_DIR) / "ift_flow" / "hierarchy.v";
  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / "ift_flow" / "hierarchy_ift_flow_tb.v";
  auto json_file = boost::filesystem::path(TESTFILES_DIR) / "ift_flow" / "hierarchy.json";
  auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / "ift_flow" / "hierarchy_ift";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  auto design = ducode::Design::parse_json(json_file);

  auto vcd_data = ducode::simulate_design(verilog_file, testbench_file);

  ducode::DesignInstance instance = ducode::DesignInstance::create_instance(design);

  //instance.write_graphviz("hierarchy.dot");

  auto top_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });
  REQUIRE(top_module != design.get_modules().end());

  const ducode::Stepsize stepsize(8u);

  auto timesteps_per_simulation_run = ducode::get_timesteps_per_simulation_run(*top_module, tag_size);
  auto number_simulation_runs = ducode::get_number_of_simulation_runs(stepsize, timesteps_per_simulation_run, vcd_data);
  params["timesteps_per_simulation_run"] = timesteps_per_simulation_run;

  auto exported_verilog = temp_dir / "ift_design.v";
  auto exported_testbench = temp_dir / "ift_testbench.v";
  std::vector<boost::filesystem::path> exported_tags_files;
  for (uint32_t i = 0; i < number_simulation_runs; ++i) {
    boost::filesystem::path tag_file = temp_dir / ("vcd_input_" + std::to_string(i) + ".txt");
    exported_tags_files.push_back(tag_file);
  }
  design.write_verilog(exported_verilog, params);

  const VCDScope* root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));
  testbench.set_tag_generator(std::make_unique<ducode::FullResolutionTagGenerator>(top_module->get_input_ports(), vcd_data));

  std::vector<VCDTime> tags_times;
  for (auto time = 0u; time <= gsl::narrow<std::size_t>(vcd_data->get_timestamps().back()); time += stepsize.value()) {
    tags_times.emplace_back(time);
  }

  testbench.write_verilog(exported_testbench, params);

  std::vector<ExportedTagsFile> exported_tags;

  /// here add loop which runs the simulation again for all created tag_map files.
  for (uint32_t i = 0; i < number_simulation_runs; ++i) {
    // For the ith batch of timestamps
    uint32_t items_per_batch = timesteps_per_simulation_run;
    uint32_t start_idx = i * items_per_batch;
    uint32_t end_idx = std::min(((i + 1) * items_per_batch), gsl::narrow<uint32_t>(tags_times.size()));

    std::vector<VCDTime> batch(tags_times.begin() + start_idx,
                               tags_times.begin() + end_idx);

    exported_tags.emplace_back(ExportedTagsFile{.time_points = batch, .exported_tags_file_path = exported_tags_files[i]});
  }

  testbench.write_tags(exported_tags, params);

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;

  for (const auto& exported_tags: exported_tags_files) {
    /// IFT simulation
    spdlog::info("Simulating Exported Design + Testbench");
    auto data = ducode::simulate_design(exported_verilog, exported_testbench, exported_tags);
    spdlog::info("End of Simulation");
    vcd_data_vector.emplace_back(data);
  }
  std::vector<const VCDScope*> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto& vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  for (const auto& tags: instance.count_output_tags()) {
    spdlog::info("Output tag: {} - {}", tags.first, tags.second);
  }
}

TEST_CASE("hierarchy_ift_flow", "[ift_flow]") {
  nlohmann::json params;
  params["ift"] = true;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;

  auto verilog_file = boost::filesystem::path(TESTFILES_DIR) / "ift_flow" / "hierarchy.v";
  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / "ift_flow" / "hierarchy_ift_flow_tb.v";
  auto json_file = boost::filesystem::path(TESTFILES_DIR) / "ift_flow" / "hierarchy.json";
  auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / "ift_flow" / "hierarchy_ift";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  auto design = ducode::Design::parse_json(json_file);

  auto vcd_data = ducode::simulate_design(verilog_file, testbench_file);

  ducode::DesignInstance instance = ducode::DesignInstance::create_instance(design);

  //instance.write_graphviz("hierarchy_ift_flow.dot");

  auto top_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });
  REQUIRE(top_module != design.get_modules().end());

  const int stepsize = 8;

  auto timesteps_per_simulation_run = ducode::get_timesteps_per_simulation_run(*top_module, tag_size);
  auto number_simulation_runs = ducode::get_number_of_simulation_runs(stepsize, timesteps_per_simulation_run, vcd_data);
  params["timesteps_per_simulation_run"] = timesteps_per_simulation_run;

  auto exported_verilog = temp_dir / "ift_design.v";
  auto exported_testbench = temp_dir / "ift_testbench.v";
  std::vector<boost::filesystem::path> exported_tags_files;
  for (uint32_t i = 0; i < number_simulation_runs; ++i) {
    boost::filesystem::path tag_file = temp_dir / ("vcd_input_" + std::to_string(i) + ".txt");
    exported_tags_files.push_back(tag_file);
  }
  design.write_verilog(exported_verilog, params);

  const VCDScope* root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));
  testbench.set_tag_generator(std::make_unique<ducode::FullResolutionTagGenerator>(top_module->get_input_ports(), vcd_data));

  std::vector<VCDTime> tags_times;
  for (auto time = 0u; time <= gsl::narrow<std::size_t>(vcd_data->get_timestamps().back()); time += stepsize) {
    tags_times.emplace_back(time);
  }

  testbench.write_verilog(exported_testbench, params);
  std::vector<ExportedTagsFile> exported_tags;

  /// here add loop which runs the simulation again for all created tag_map files.
  for (uint32_t i = 0; i < number_simulation_runs; ++i) {
    // For the ith batch of timestamps
    uint32_t items_per_batch = timesteps_per_simulation_run;
    uint32_t start_idx = i * items_per_batch;
    uint32_t end_idx = std::min(((i + 1) * items_per_batch), gsl::narrow<uint32_t>(tags_times.size()));

    std::vector<VCDTime> batch(tags_times.begin() + start_idx,
                               tags_times.begin() + end_idx);

    exported_tags.emplace_back(batch, exported_tags_files[i]);
  }
  testbench.write_tags(exported_tags, params);

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;

  for (const auto& exported_tags: exported_tags_files) {
    /// IFT simulation
    spdlog::info("Simulating Exported Design + Testbench");
    auto data = ducode::simulate_design(exported_verilog, exported_testbench, exported_tags);
    spdlog::info("End of Simulation");
    vcd_data_vector.emplace_back(data);
  }
  std::vector<const VCDScope*> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto& vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  //instance.write_graphviz("hierarchy.dot");
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

  auto find_edge = [&](std::string_view edge_name, std::vector<std::string>& hierarchy) -> ducode::instantiation_graph::edge_descriptor {
    for (const auto& edge: subrange(edges(instance.m_graph))) {
      if (instance.m_graph.m_vertices[edge.m_target].m_property.name == edge_name && instance.m_graph.m_vertices[edge.m_target].m_property.hierarchy == hierarchy) {
        return edge;
      }
    }
    throw std::runtime_error("name not found!");
  };

  std::vector<std::string> hierarchy_vec = {"hierarchy", "m1", "s1"};
  std::vector<ducode::SignalValues> values = instance.get_tag_values(find_edge("S_c", hierarchy_vec));
  CHECK(values[0].m_timed_signal_values[0].value.get_value_vector()[0] == ducode::SignalBit::BIT_1);
  CHECK(values[0].m_timed_signal_values[0].value.get_value_vector()[1] == ducode::SignalBit::BIT_0);

  CHECK(values[0].m_timed_signal_values[1].value.get_value_vector()[0] == ducode::SignalBit::BIT_1);
  CHECK(values[0].m_timed_signal_values[1].value.get_value_vector()[1] == ducode::SignalBit::BIT_1);
  CHECK(values[0].m_timed_signal_values[1].value.get_value_vector()[2] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[1].value.get_value_vector()[3] == ducode::SignalBit::BIT_0);

  hierarchy_vec = {"hierarchy", "m1", "s2"};
  values = instance.get_tag_values(find_edge("S_c", hierarchy_vec));
  CHECK(values[0].m_timed_signal_values[0].value.get_value_vector()[0] == ducode::SignalBit::BIT_1);
  CHECK(values[0].m_timed_signal_values[0].value.get_value_vector()[1] == ducode::SignalBit::BIT_0);

  CHECK(values[0].m_timed_signal_values[1].value.get_value_vector()[0] == ducode::SignalBit::BIT_1);
  CHECK(values[0].m_timed_signal_values[1].value.get_value_vector()[1] == ducode::SignalBit::BIT_1);
  CHECK(values[0].m_timed_signal_values[1].value.get_value_vector()[2] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[1].value.get_value_vector()[3] == ducode::SignalBit::BIT_0);

  hierarchy_vec = {"hierarchy"};
  values = instance.get_tag_values(find_edge("hierarchy_c", hierarchy_vec));
  CHECK(values[0].m_timed_signal_values[0].value.get_value_vector()[0] == ducode::SignalBit::BIT_1);
  CHECK(values[0].m_timed_signal_values[0].value.get_value_vector()[1] == ducode::SignalBit::BIT_0);

  CHECK(values[0].m_timed_signal_values[1].value.get_value_vector()[0] == ducode::SignalBit::BIT_1);
  CHECK(values[0].m_timed_signal_values[1].value.get_value_vector()[1] == ducode::SignalBit::BIT_1);
  CHECK(values[0].m_timed_signal_values[1].value.get_value_vector()[2] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[1].value.get_value_vector()[3] == ducode::SignalBit::BIT_0);

  CHECK(values[0].m_timed_signal_values[3].value.get_value_vector()[0] == ducode::SignalBit::BIT_1);
  CHECK(values[0].m_timed_signal_values[3].value.get_value_vector()[1] == ducode::SignalBit::BIT_1);
  CHECK(values[0].m_timed_signal_values[3].value.get_value_vector()[2] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[3].value.get_value_vector()[3] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[3].value.get_value_vector()[4] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[3].value.get_value_vector()[5] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[3].value.get_value_vector()[6] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[3].value.get_value_vector()[7] == ducode::SignalBit::BIT_0);
}

TEST_CASE("secureAccess", "[ift_flow]") {
  nlohmann::json params;
  params["ift"] = true;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;

  auto verilog_file = boost::filesystem::path(TESTFILES_DIR) / "cwe1302_Issue" / "secureAccess.v";
  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / "cwe1302_Issue" / "secureAccess_tb.v";
  auto json_file = boost::filesystem::path(TESTFILES_DIR) / "cwe1302_Issue" / "secureAccess.json";
  /// auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / "ift_flow" / "hierarchy_ift";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  auto design = ducode::Design::parse_json(json_file);

  auto vcd_data = ducode::simulate_design(verilog_file, testbench_file);

  ducode::DesignInstance instance = ducode::DesignInstance::create_instance(design);

  //instance.write_graphviz("secureAccess.dot");

  auto top_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });
  REQUIRE(top_module != design.get_modules().end());

  const int stepsize = 10;

  auto timesteps_per_simulation_run = ducode::get_timesteps_per_simulation_run(*top_module, tag_size);
  auto number_simulation_runs = ducode::get_number_of_simulation_runs(stepsize, timesteps_per_simulation_run, vcd_data);
  params["timesteps_per_simulation_run"] = timesteps_per_simulation_run;

  auto exported_verilog = temp_dir / "ift_design.v";
  auto exported_testbench = temp_dir / "ift_testbench.v";
  std::vector<boost::filesystem::path> exported_tags_files;
  for (uint32_t i = 0; i < number_simulation_runs; ++i) {
    boost::filesystem::path tag_file = temp_dir / ("vcd_input_" + std::to_string(i) + ".txt");
    exported_tags_files.push_back(tag_file);
  }
  design.write_verilog(exported_verilog, params);

  const VCDScope* root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));
  testbench.set_tag_generator(std::make_unique<ducode::FullResolutionTagGenerator>(top_module->get_input_ports(), vcd_data));

  std::vector<VCDTime> tags_times;
  for (auto time = 0u; time <= gsl::narrow<std::size_t>(vcd_data->get_timestamps().back()); time += stepsize) {
    tags_times.emplace_back(time);
  }

  testbench.write_verilog(exported_testbench, params);
  std::vector<ExportedTagsFile> exported_tags;

  /// here add loop which runs the simulation again for all created tag_map files.
  for (uint32_t i = 0; i < number_simulation_runs; ++i) {
    // For the ith batch of timestamps
    uint32_t items_per_batch = timesteps_per_simulation_run;
    uint32_t start_idx = i * items_per_batch;
    uint32_t end_idx = std::min(((i + 1) * items_per_batch), gsl::narrow<uint32_t>(tags_times.size()));

    std::vector<VCDTime> batch(tags_times.begin() + start_idx,
                               tags_times.begin() + end_idx);

    exported_tags.emplace_back(batch, exported_tags_files[i]);
  }
  testbench.write_tags(exported_tags, params);

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;

  for (const auto& exported_tags: exported_tags_files) {
    /// IFT simulation
    spdlog::info("Simulating Exported Design + Testbench");
    auto data = ducode::simulate_design(exported_verilog, exported_testbench, exported_tags);
    spdlog::info("End of Simulation");
    vcd_data_vector.emplace_back(data);
  }
  std::vector<const VCDScope*> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto& vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  //instance.write_graphviz("hierarchy.dot");
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

/* TEST_CASE("secureAccess_ift_tagging", "[ift_flow]") {
  nlohmann::json params;
  params["ift"] = true;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;

  auto verilog_file = boost::filesystem::path(TESTFILES_DIR) / "cwe1302_Issue" / "secureAccess_ift_tagging.v";
  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / "cwe1302_Issue" / "secureAccess_ift_tagging_tb.v";
  auto json_file = boost::filesystem::path(TESTFILES_DIR) / "cwe1302_Issue" / "secureAccess_ift_tagging.json";
  /// auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / "ift_flow" / "hierarchy_ift";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  auto design = ducode::Design::parse_json(json_file);

  auto vcd_data = ducode::simulate_design(verilog_file, testbench_file);

  ducode::DesignInstance instance = ducode::DesignInstance::create_instance(design);

  instance.write_graphviz("secureAccess_ift_tagging.dot");

  auto top_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });
  REQUIRE(top_module != design.get_modules().end());

  const int stepsize = 10;

  auto timesteps_per_simulation_run = ducode::get_timesteps_per_simulation_run(*top_module, tag_size);
  auto number_simulation_runs = ducode::get_number_of_simulation_runs(stepsize, timesteps_per_simulation_run, vcd_data);
  params["timesteps_per_simulation_run"] = timesteps_per_simulation_run;

  auto exported_verilog = temp_dir / "ift_design.v";
  auto exported_testbench = temp_dir / "ift_testbench.v";
  std::vector<boost::filesystem::path> exported_tags_files;
  for (uint32_t i = 0; i < number_simulation_runs; ++i) {
    boost::filesystem::path tag_file = temp_dir / ("vcd_input_" + std::to_string(i) + ".txt");
    exported_tags_files.push_back(tag_file);
  }
  design.write_verilog(exported_verilog, params);

  const VCDScope* root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));
  testbench.set_tag_generator(std::make_unique<ducode::FullResolutionTagGenerator>(top_module->get_input_ports(), vcd_data));

  std::vector<VCDTime> tags_times;
  for (auto time = 0u; time <= gsl::narrow<std::size_t>(vcd_data->get_timestamps().back()); time += stepsize) {
    tags_times.emplace_back(time);
  }

  testbench.write_verilog(exported_testbench, params);
  std::vector<ExportedTagsFile> exported_tags;

  /// here add loop which runs the simulation again for all created tag_map files.
  for (uint32_t i = 0; i < number_simulation_runs; ++i) {
    // For the ith batch of timestamps
    uint32_t items_per_batch = timesteps_per_simulation_run;
    uint32_t start_idx = i * items_per_batch;
    uint32_t end_idx = std::min(((i + 1) * items_per_batch), gsl::narrow<uint32_t>(tags_times.size()));

    std::vector<VCDTime> batch(tags_times.begin() + start_idx,
                               tags_times.begin() + end_idx);

    exported_tags.emplace_back(batch, exported_tags_files[i]);
  }
  testbench.write_tags(exported_tags, params);

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;

  for (const auto& exported_tags: exported_tags_files) {
    /// IFT simulation
    spdlog::info("Simulating Exported Design + Testbench");
    auto data = ducode::simulate_design(exported_verilog, exported_testbench, exported_tags);
    spdlog::info("End of Simulation");
    vcd_data_vector.emplace_back(data);
  }
  std::vector<const VCDScope*> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto& vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  instance.write_graphviz("hierarchysecureAccess_ift_tagging1.dot");
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
}*/

TEST_CASE("secureAccess_moduleTagging", "[ift_flow]") {
  nlohmann::json params;
  params["ift"] = true;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;
  params["tagged_module_name"] = "assignerCoreSec";

  auto verilog_file = boost::filesystem::path(TESTFILES_DIR) / "cwe1302_Issue" / "secureAccess.v";
  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / "cwe1302_Issue" / "secureAccess_tb.v";
  auto json_file = boost::filesystem::path(TESTFILES_DIR) / "cwe1302_Issue" / "secureAccess.json";
  /// auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / "ift_flow" / "hierarchy_ift";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  auto design = ducode::Design::parse_json(json_file);

  auto vcd_data = ducode::simulate_design(verilog_file, testbench_file);

  ducode::DesignInstance instance = ducode::DesignInstance::create_instance(design);

  //instance.write_graphviz("secureAccess.dot");

  auto top_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });
  REQUIRE(top_module != design.get_modules().end());

  auto tagged_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == "assignerCoreSec"; });
  REQUIRE(tagged_module != design.get_modules().end());

  const int stepsize = 10;

  auto timesteps_per_simulation_run = ducode::get_timesteps_per_simulation_run_internal_module_tagging(*tagged_module, tag_size);
  auto number_simulation_runs = ducode::get_number_of_simulation_runs(stepsize, timesteps_per_simulation_run, vcd_data);
  params["timesteps_per_simulation_run"] = timesteps_per_simulation_run;

  auto exported_verilog = temp_dir / "ift_design.v";
  auto exported_testbench = temp_dir / "ift_testbench.v";
  std::vector<boost::filesystem::path> exported_tags_files;
  for (uint32_t i = 0; i < number_simulation_runs; ++i) {
    boost::filesystem::path tag_file = temp_dir / ("vcd_input_" + std::to_string(i) + ".txt");
    exported_tags_files.push_back(tag_file);
  }
  design.write_verilog(exported_verilog, params);

  /*const VCDScope* root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));
  testbench.set_tag_generator(std::make_unique<ducode::FullResolutionTagGenerator>(top_module->get_input_ports(), vcd_data));*/

  const VCDScope* root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));
  testbench.set_tag_generator(std::make_unique<ducode::FullResolutionTagGenerator>(tagged_module->get_output_ports(), vcd_data));

  std::vector<VCDTime> tags_times;
  for (auto time = 0u; time <= gsl::narrow<std::size_t>(vcd_data->get_timestamps().back()); time += stepsize) {
    tags_times.emplace_back(time);
  }

  testbench.write_verilog(exported_testbench, params);
  std::vector<ExportedTagsFile> exported_tags;

  /// here add loop which runs the simulation again for all created tag_map files.
  for (uint32_t i = 0; i < number_simulation_runs; ++i) {
    // For the ith batch of timestamps
    uint32_t items_per_batch = timesteps_per_simulation_run;
    uint32_t start_idx = i * items_per_batch;
    uint32_t end_idx = std::min(((i + 1) * items_per_batch), gsl::narrow<uint32_t>(tags_times.size()));

    std::vector<VCDTime> batch(tags_times.begin() + start_idx,
                               tags_times.begin() + end_idx);

    exported_tags.emplace_back(batch, exported_tags_files[i]);
  }
  testbench.write_tags(exported_tags, params);

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;

  for (const auto& exported_tags: exported_tags_files) {
    /// IFT simulation
    spdlog::info("Simulating Exported Design + Testbench");
    auto data = ducode::simulate_design(exported_verilog, exported_testbench, exported_tags);
    spdlog::info("End of Simulation");
    vcd_data_vector.emplace_back(data);
  }
  std::vector<const VCDScope*> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto& vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  //instance.write_graphviz("hierarchy.dot");
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

  auto find_edge = [&](std::string_view edge_name, std::vector<std::string>& hierarchy) -> ducode::instantiation_graph::edge_descriptor {
    for (const auto& edge: subrange(edges(instance.m_graph))) {
      if (instance.m_graph.m_vertices[edge.m_target].m_property.name == edge_name && instance.m_graph.m_vertices[edge.m_target].m_property.hierarchy == hierarchy) {
        return edge;
      }
    }
    throw std::runtime_error("name not found!");
  };

  std::vector<std::string> hierarchy_vec = {"secureInterface", "sm", "ac"};
  std::vector<ducode::SignalValues> values = instance.get_tag_values(find_edge("assignerCoreSec_grantAccessUsecase_o", hierarchy_vec));

  /*for (long unsigned int loopVar_i = 0; loopVar_i < (values[0][0].size() - 1); loopVar_i++) {
    //for (auto loopVar_j = 0; loopVar_j < (values[0][0][loopVar_i].value.get_value_vector().size()); loopVar_j = loopVar_j+2) {
    CHECK(values[0][0][loopVar_i].value.get_value_vector()[0] == VCDBit::VCD_1);
    //}
  }*/

  CHECK(values[0].m_timed_signal_values[0].value.get_value_vector()[0] == ducode::SignalBit::BIT_1);
  CHECK(values[0].m_timed_signal_values[0].value.get_value_vector()[1] == ducode::SignalBit::BIT_0);

  CHECK(values[0].m_timed_signal_values[1].value.get_value_vector()[0] == ducode::SignalBit::BIT_1);
  CHECK(values[0].m_timed_signal_values[1].value.get_value_vector()[1] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[1].value.get_value_vector()[2] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[1].value.get_value_vector()[3] == ducode::SignalBit::BIT_0);

  CHECK(values[0].m_timed_signal_values[2].value.get_value_vector()[0] == ducode::SignalBit::BIT_1);
  CHECK(values[0].m_timed_signal_values[2].value.get_value_vector()[1] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[2].value.get_value_vector()[2] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[2].value.get_value_vector()[3] == ducode::SignalBit::BIT_0);

  CHECK(values[0].m_timed_signal_values[3].value.get_value_vector()[0] == ducode::SignalBit::BIT_1);
  CHECK(values[0].m_timed_signal_values[3].value.get_value_vector()[1] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[3].value.get_value_vector()[2] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[3].value.get_value_vector()[3] == ducode::SignalBit::BIT_0);

  CHECK(values[0].m_timed_signal_values[4].value.get_value_vector()[0] == ducode::SignalBit::BIT_1);
  CHECK(values[0].m_timed_signal_values[4].value.get_value_vector()[1] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[4].value.get_value_vector()[2] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[4].value.get_value_vector()[3] == ducode::SignalBit::BIT_0);

  hierarchy_vec = {"secureInterface", "sm", "sc", "ec"};
  values = instance.get_tag_values(find_edge("AESCoreSec_secretData_i", hierarchy_vec));

  /*for (long unsigned int loopVar_i = 0; loopVar_i < (values[0][0].size() - 1); loopVar_i++) {
    //for (auto loopVar_j = 0; loopVar_j < (values[0][0][loopVar_i].value.get_value_vector().size()); loopVar_j = loopVar_j+2) {
    CHECK(values[0][0][loopVar_i].value.get_value_vector()[0] == VCDBit::VCD_1);
    //}
  }*/

  CHECK(values[0].m_timed_signal_values[0].value.get_value_vector()[0] == ducode::SignalBit::BIT_1);
  CHECK(values[0].m_timed_signal_values[0].value.get_value_vector()[1] == ducode::SignalBit::BIT_0);

  CHECK(values[0].m_timed_signal_values[1].value.get_value_vector()[0] == ducode::SignalBit::BIT_1);
  CHECK(values[0].m_timed_signal_values[1].value.get_value_vector()[1] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[1].value.get_value_vector()[2] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[1].value.get_value_vector()[3] == ducode::SignalBit::BIT_0);

  CHECK(values[0].m_timed_signal_values[2].value.get_value_vector()[0] == ducode::SignalBit::BIT_1);
  CHECK(values[0].m_timed_signal_values[2].value.get_value_vector()[1] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[2].value.get_value_vector()[2] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[2].value.get_value_vector()[3] == ducode::SignalBit::BIT_0);

  CHECK(values[0].m_timed_signal_values[3].value.get_value_vector()[0] == ducode::SignalBit::BIT_1);
  CHECK(values[0].m_timed_signal_values[3].value.get_value_vector()[1] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[3].value.get_value_vector()[2] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[3].value.get_value_vector()[3] == ducode::SignalBit::BIT_0);

  CHECK(values[0].m_timed_signal_values[4].value.get_value_vector()[0] == ducode::SignalBit::BIT_1);
  CHECK(values[0].m_timed_signal_values[4].value.get_value_vector()[1] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[4].value.get_value_vector()[2] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[4].value.get_value_vector()[3] == ducode::SignalBit::BIT_0);

  hierarchy_vec = {"secureInterface", "sm", "sc", "hc"};
  values = instance.get_tag_values(find_edge("RSACoreSec_secretData_i", hierarchy_vec));

  /*for (long unsigned int loopVar_i = 0; loopVar_i < (values[0][0].size() - 1); loopVar_i++) {
    CHECK(values[0][0][loopVar_i].value.get_value_vector()[0] == VCDBit::VCD_1);
  }*/

  CHECK(values[0].m_timed_signal_values[0].value.get_value_vector()[0] == ducode::SignalBit::BIT_1);
  CHECK(values[0].m_timed_signal_values[0].value.get_value_vector()[1] == ducode::SignalBit::BIT_0);

  CHECK(values[0].m_timed_signal_values[1].value.get_value_vector()[0] == ducode::SignalBit::BIT_1);
  CHECK(values[0].m_timed_signal_values[1].value.get_value_vector()[1] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[1].value.get_value_vector()[2] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[1].value.get_value_vector()[3] == ducode::SignalBit::BIT_0);

  CHECK(values[0].m_timed_signal_values[2].value.get_value_vector()[0] == ducode::SignalBit::BIT_1);
  CHECK(values[0].m_timed_signal_values[2].value.get_value_vector()[1] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[2].value.get_value_vector()[2] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[2].value.get_value_vector()[3] == ducode::SignalBit::BIT_0);

  CHECK(values[0].m_timed_signal_values[3].value.get_value_vector()[0] == ducode::SignalBit::BIT_1);
  CHECK(values[0].m_timed_signal_values[3].value.get_value_vector()[1] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[3].value.get_value_vector()[2] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[3].value.get_value_vector()[3] == ducode::SignalBit::BIT_0);

  CHECK(values[0].m_timed_signal_values[4].value.get_value_vector()[0] == ducode::SignalBit::BIT_1);
  CHECK(values[0].m_timed_signal_values[4].value.get_value_vector()[1] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[4].value.get_value_vector()[2] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[4].value.get_value_vector()[3] == ducode::SignalBit::BIT_0);
}

TEST_CASE("secureAccess_moduleTagging_Fix", "[ift_flow]") {
  nlohmann::json params;
  params["ift"] = true;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;
  params["tagged_module_name"] = "assignerCoreSec";

  auto verilog_file = boost::filesystem::path(TESTFILES_DIR) / "cwe1302_Fix" / "secureAccess.v";
  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / "cwe1302_Fix" / "secureAccess_tb.v";
  auto json_file = boost::filesystem::path(TESTFILES_DIR) / "cwe1302_Fix" / "secureAccess.json";
  /// auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / "ift_flow" / "hierarchy_ift";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  auto design = ducode::Design::parse_json(json_file);

  auto vcd_data = ducode::simulate_design(verilog_file, testbench_file);

  ducode::DesignInstance instance = ducode::DesignInstance::create_instance(design);

  //instance.write_graphviz("secureAccess.dot");

  auto top_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });
  REQUIRE(top_module != design.get_modules().end());

  auto tagged_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == "assignerCoreSec"; });
  REQUIRE(tagged_module != design.get_modules().end());

  const int stepsize = 10;

  auto timesteps_per_simulation_run = ducode::get_timesteps_per_simulation_run_internal_module_tagging(*tagged_module, tag_size);
  auto number_simulation_runs = ducode::get_number_of_simulation_runs(stepsize, timesteps_per_simulation_run, vcd_data);
  params["timesteps_per_simulation_run"] = timesteps_per_simulation_run;

  auto exported_verilog = temp_dir / "ift_design.v";
  auto exported_testbench = temp_dir / "ift_testbench.v";
  std::vector<boost::filesystem::path> exported_tags_files;
  for (uint32_t i = 0; i < number_simulation_runs; ++i) {
    boost::filesystem::path tag_file = temp_dir / ("vcd_input_" + std::to_string(i) + ".txt");
    exported_tags_files.push_back(tag_file);
  }
  design.write_verilog(exported_verilog, params);

  /*const VCDScope* root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));
  testbench.set_tag_generator(std::make_unique<ducode::FullResolutionTagGenerator>(top_module->get_input_ports(), vcd_data));*/

  const VCDScope* root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));
  testbench.set_tag_generator(std::make_unique<ducode::FullResolutionTagGenerator>(tagged_module->get_output_ports(), vcd_data));

  std::vector<VCDTime> tags_times;
  for (auto time = 0u; time <= gsl::narrow<std::size_t>(vcd_data->get_timestamps().back()); time += stepsize) {
    tags_times.emplace_back(time);
  }

  testbench.write_verilog(exported_testbench, params);
  std::vector<ExportedTagsFile> exported_tags;

  /// here add loop which runs the simulation again for all created tag_map files.
  for (uint32_t i = 0; i < number_simulation_runs; ++i) {
    // For the ith batch of timestamps
    uint32_t items_per_batch = timesteps_per_simulation_run;
    uint32_t start_idx = i * items_per_batch;
    uint32_t end_idx = std::min(((i + 1) * items_per_batch), gsl::narrow<uint32_t>(tags_times.size()));

    std::vector<VCDTime> batch(tags_times.begin() + start_idx,
                               tags_times.begin() + end_idx);

    exported_tags.emplace_back(batch, exported_tags_files[i]);
  }
  testbench.write_tags(exported_tags, params);

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;

  for (const auto& exported_tags: exported_tags_files) {
    /// IFT simulation
    spdlog::info("Simulating Exported Design + Testbench");
    auto data = ducode::simulate_design(exported_verilog, exported_testbench, exported_tags);
    spdlog::info("End of Simulation");
    vcd_data_vector.emplace_back(data);
  }
  std::vector<const VCDScope*> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto& vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  //instance.write_graphviz("hierarchy.dot");
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

  auto find_edge = [&](std::string_view edge_name, std::vector<std::string>& hierarchy) -> ducode::instantiation_graph::edge_descriptor {
    for (const auto& edge: subrange(edges(instance.m_graph))) {
      if (instance.m_graph.m_vertices[edge.m_target].m_property.name == edge_name && instance.m_graph.m_vertices[edge.m_target].m_property.hierarchy == hierarchy) {
        return edge;
      }
    }
    throw std::runtime_error("name not found!");
  };

  std::vector<std::string> hierarchy_vec = {"secureInterface", "sm", "ac"};
  std::vector<ducode::SignalValues> values = instance.get_tag_values(find_edge("assignerCoreSec_grantAccessUsecase_o", hierarchy_vec));

  /*for (long unsigned int loopVar_i = 0; loopVar_i < (values[0][0].size() - 1); loopVar_i++) {
    //for (auto loopVar_j = 0; loopVar_j < (values[0][0][loopVar_i].value.get_value_vector().size()); loopVar_j = loopVar_j+2) {
    CHECK(values[0][0][loopVar_i].value.get_value_vector()[0] == VCDBit::VCD_1);
    //}
  }*/

  CHECK(values[0].m_timed_signal_values[0].value.get_value_vector()[0] == ducode::SignalBit::BIT_1);
  CHECK(values[0].m_timed_signal_values[0].value.get_value_vector()[1] == ducode::SignalBit::BIT_0);

  CHECK(values[0].m_timed_signal_values[1].value.get_value_vector()[0] == ducode::SignalBit::BIT_1);
  CHECK(values[0].m_timed_signal_values[1].value.get_value_vector()[1] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[1].value.get_value_vector()[2] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[1].value.get_value_vector()[3] == ducode::SignalBit::BIT_0);

  CHECK(values[0].m_timed_signal_values[2].value.get_value_vector()[0] == ducode::SignalBit::BIT_1);
  CHECK(values[0].m_timed_signal_values[2].value.get_value_vector()[1] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[2].value.get_value_vector()[2] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[2].value.get_value_vector()[3] == ducode::SignalBit::BIT_0);

  CHECK(values[0].m_timed_signal_values[3].value.get_value_vector()[0] == ducode::SignalBit::BIT_1);
  CHECK(values[0].m_timed_signal_values[3].value.get_value_vector()[1] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[3].value.get_value_vector()[2] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[3].value.get_value_vector()[3] == ducode::SignalBit::BIT_0);

  CHECK(values[0].m_timed_signal_values[4].value.get_value_vector()[0] == ducode::SignalBit::BIT_1);
  CHECK(values[0].m_timed_signal_values[4].value.get_value_vector()[1] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[4].value.get_value_vector()[2] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[4].value.get_value_vector()[3] == ducode::SignalBit::BIT_0);

  hierarchy_vec = {"secureInterface", "sm", "sc", "ec"};
  values = instance.get_tag_values(find_edge("AESCoreSec_secretData_i", hierarchy_vec));

  /*for (long unsigned int loopVar_i = 0; loopVar_i < (values[0][0].size() - 1); loopVar_i++) {
    //for (auto loopVar_j = 0; loopVar_j < (values[0][0][loopVar_i].value.get_value_vector().size()); loopVar_j = loopVar_j+2) {
    CHECK(values[0][0][loopVar_i].value.get_value_vector()[0] == VCDBit::VCD_1);
    //}
  }*/

  CHECK(values[0].m_timed_signal_values[0].value.get_value_vector()[0] == ducode::SignalBit::BIT_0);

  hierarchy_vec = {"secureInterface", "sm", "sc", "hc"};
  values = instance.get_tag_values(find_edge("RSACoreSec_secretData_i", hierarchy_vec));

  /*for (long unsigned int loopVar_i = 0; loopVar_i < (values[0][0].size() - 1); loopVar_i++) {
    CHECK(values[0][0][loopVar_i].value.get_value_vector()[0] == VCDBit::VCD_1);
  }*/

  CHECK(values[0].m_timed_signal_values[0].value.get_value_vector()[0] == ducode::SignalBit::BIT_0);

  CHECK(values[0].m_timed_signal_values[1].value.get_value_vector()[0] == ducode::SignalBit::BIT_1);
  CHECK(values[0].m_timed_signal_values[1].value.get_value_vector()[1] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[1].value.get_value_vector()[2] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[1].value.get_value_vector()[3] == ducode::SignalBit::BIT_0);

  CHECK(values[0].m_timed_signal_values[2].value.get_value_vector()[0] == ducode::SignalBit::BIT_1);
  CHECK(values[0].m_timed_signal_values[2].value.get_value_vector()[1] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[2].value.get_value_vector()[2] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[2].value.get_value_vector()[3] == ducode::SignalBit::BIT_0);

  CHECK(values[0].m_timed_signal_values[3].value.get_value_vector()[0] == ducode::SignalBit::BIT_1);
  CHECK(values[0].m_timed_signal_values[3].value.get_value_vector()[1] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[3].value.get_value_vector()[2] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[3].value.get_value_vector()[3] == ducode::SignalBit::BIT_0);

  CHECK(values[0].m_timed_signal_values[4].value.get_value_vector()[0] == ducode::SignalBit::BIT_1);
  CHECK(values[0].m_timed_signal_values[4].value.get_value_vector()[1] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[4].value.get_value_vector()[2] == ducode::SignalBit::BIT_0);
  CHECK(values[0].m_timed_signal_values[4].value.get_value_vector()[3] == ducode::SignalBit::BIT_0);
}

TEST_CASE("ift_flow_fft16", "[ift_flow]") {
  nlohmann::json params;
  params["ift"] = true;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;

  const int stepsize = 2000000;
  auto json_file = std::filesystem::path(TESTFILES_DIR) / "fft" / "fft16.json";
  auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / "fft" / "fft16.vcd";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  auto design = ducode::Design::parse_json(json_file);
  VCDFileParser vcd_obj;
  auto vcd_data = vcd_obj.parse_file(vcd_reference_file_path.string());

  ducode::DesignInstance instance = ducode::DesignInstance::create_instance(design);

  //instance.write_graphviz("ift_flow_fft16.dot");

  auto top_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });
  REQUIRE(top_module != design.get_modules().end());

  auto timesteps_per_simulation_run = tag_size / top_module->get_input_names().size();
  timesteps_per_simulation_run = std::max<size_t>(timesteps_per_simulation_run, 1);
  auto number_simulation_runs = ducode::get_number_of_simulation_runs(stepsize, timesteps_per_simulation_run, vcd_data);
  params["timesteps_per_simulation_run"] = timesteps_per_simulation_run;

  auto exported_verilog = temp_dir / "ift_design.v";
  auto exported_testbench = temp_dir / "ift_testbench.v";
  std::vector<boost::filesystem::path> exported_tags_files;
  for (uint32_t i = 0; i < number_simulation_runs; ++i) {
    boost::filesystem::path tag_file = temp_dir / ("vcd_input_" + std::to_string(i) + ".txt");
    exported_tags_files.push_back(tag_file);
  }
  design.write_verilog(exported_verilog, params);

  const VCDScope* root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));
  testbench.set_tag_generator(std::make_unique<ducode::FullResolutionTagGenerator>(top_module->get_input_ports(), vcd_data));

  std::vector<VCDTime> tags_times;
  for (auto time = 0u; time <= gsl::narrow<std::size_t>(vcd_data->get_timestamps().back()); time += stepsize) {
    tags_times.emplace_back(time);
  }

  testbench.write_verilog(exported_testbench, params);
  std::vector<ExportedTagsFile> exported_tags;

  /// here add loop which runs the simulation again for all created tag_map files.
  for (uint32_t i = 0; i < number_simulation_runs; ++i) {
    // For the ith batch of timestamps
    uint32_t items_per_batch = timesteps_per_simulation_run;
    uint32_t start_idx = i * items_per_batch;
    uint32_t end_idx = std::min(((i + 1) * items_per_batch), gsl::narrow<uint32_t>(tags_times.size()));

    std::vector<VCDTime> batch(tags_times.begin() + start_idx,
                               tags_times.begin() + end_idx);

    exported_tags.emplace_back(batch, exported_tags_files[i]);
  }
  testbench.write_tags(exported_tags, params);

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;

  for (const auto& exported_tags: exported_tags_files) {
    /// IFT simulation
    spdlog::info("Simulating Exported Design + Testbench");
    auto data = ducode::simulate_design(exported_verilog, exported_testbench, exported_tags);
    spdlog::info("End of Simulation");
    vcd_data_vector.emplace_back(data);
  }
  std::vector<const VCDScope*> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto& vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  bool contains_x = false;

  for (auto [edge_it, edge_end] = boost::edges(instance.m_graph); edge_it != edge_end; ++edge_it) {
    if (instance.m_graph[*edge_it].net_ptr != nullptr) {
      auto tag_values = instance.get_tag_values(*edge_it);
      for (const auto& tag_vector: tag_values) {
        for (const auto& tag_value: tag_vector.m_timed_signal_values) {
          for (const auto& tag_bit: tag_value.value.get_value_vector()) {
            if (tag_bit == ducode::SignalBit::BIT_X) {
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
