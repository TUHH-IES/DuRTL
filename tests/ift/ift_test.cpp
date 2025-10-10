/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/instantiation_graph.hpp>
#include <ducode/instantiation_graph_traits.hpp>
#include <ducode/signals_data_manager.hpp>
#include <ducode/tag_generator.hpp>
#include <ducode/testbench.hpp>
#include <ducode/utility/VCD_utility.hpp>
#include <ducode/utility/iverilog_wrapper.hpp>
#include <ducode/utility/simulation.hpp>
#include <ducode/utility/subrange.hpp>
#include <ducode/utility/types.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/detail/adjacency_list.hpp>
//#include <boost/graph/graphviz.hpp>
//#include <boost/graph/properties.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <fmt/core.h>
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
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

// testet ob mindestens eine 1 an einem output eines hardware designs ankommen und das kein x-value an einem output ankommt
TEST_CASE("ift_test", "[ift_test]") {
  auto test_parameters = GENERATE(std::make_tuple("aes_module_t", 1));
  const auto& [design_name, stepsize] = test_parameters;

  nlohmann::json params;

  params["ift"] = false;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;
  params["stepsize"] = stepsize;

  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / design_name / fmt::format("{}_tb.v", design_name);
  auto json_file = std::filesystem::path(TESTFILES_DIR) / design_name / fmt::format("{}.json", design_name);

  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  auto exported_verilog = temp_dir / fmt::format("ift{}.v", design_name);
  auto exported_testbench = temp_dir / fmt::format("ift{}_tb.v", design_name);

  auto design = ducode::Design::parse_json(json_file);

  //export the design without IFT
  design.write_verilog(exported_verilog, params);

  // original design Simulation
  auto vcd_data = ducode::simulate_design(exported_verilog, testbench_file);

  params["ift"] = true;

  //export the design with IFT
  design.write_verilog(exported_verilog, params);

  //get top module
  auto top_module = design.get_top_module();

  auto timesteps_per_simulation_run = get_timesteps_per_simulation_run(top_module, tag_size);
  params["timesteps_per_simulation_run"] = timesteps_per_simulation_run;

  auto number_simulation_runs = ducode::get_number_of_simulation_runs(stepsize, timesteps_per_simulation_run, vcd_data);

  spdlog::info("Running {} simulations", number_simulation_runs);

  std::vector<boost::filesystem::path> exported_tags_files = ducode::get_exported_tags_files(number_simulation_runs);

  //exporting testbench with IFT
  const VCDScope* root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));

  testbench.write_verilog(exported_testbench, params);
  testbench.set_tag_generator(
      std::make_unique<ducode::FullResolutionTagGenerator>(top_module.get_input_ports(), vcd_data));

  std::vector<VCDTime> tags_times = ducode::get_tags_times(vcd_data, stepsize);

  auto exported_tags_vec =
      ducode::get_exported_tags_vec(
          exported_tags_files, number_simulation_runs, tags_times, timesteps_per_simulation_run);

  testbench.write_tags(exported_tags_vec, params);

  spdlog::info("Simulating Exported Design + Testbench");

  auto instance = ducode::DesignInstance::create_instance(design);

  std::vector<std::shared_ptr<VCDFile>> data_vec;

  for (const auto& exported_tags: exported_tags_files) {
    auto data = ducode::simulate_design(exported_verilog, exported_testbench, exported_tags);
    data_vec.emplace_back(data);
  }
  std::vector<const VCDScope*> vcd_root_scopes;
  vcd_root_scopes.reserve(data_vec.size());
  for (auto& vcd_file: data_vec) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(data_vec, vcd_root_scopes, data_vec.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  bool contains_tags = false;
  bool contains_x = false;


  for (auto& vertex: instance.m_graph.m_vertices) {
    if (vertex.m_property.type == "module_port") {
      for (const auto& edge: ducode::subrange(boost::edges(instance.m_graph))) {
        for (const auto& tag_vector: instance.get_tag_values(edge)) {
          for (const auto& tag_value: tag_vector.m_timed_signal_values) {
            for (const auto& tgs: tag_value.value.get_value_vector()) {
              if (tgs == ducode::SignalBit::BIT_1) {
                contains_tags = true;
                break;
              }
            }
            for (const auto& tgs: tag_value.value.get_value_vector()) {
              if (tgs == ducode::SignalBit::BIT_X) {
                contains_x = true;
                break;
              }
            }
          }
        }
      }
    }
  }
  CHECK(contains_tags);
  CHECK(!contains_x);
}

TEST_CASE("ift_test_fft_apprx", "[ift_test]") {
  nlohmann::json params;
  params["ift"] = true;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;

  const int stepsize = 500;
  auto json_file = std::filesystem::path(TESTFILES_DIR) / "fft_appx_add" / "fft16_apx.json";
  auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / "fft_appx_add" / "tb_fft16_ofdm_apx.vcd";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  auto design = ducode::Design::parse_json(json_file);
  VCDFileParser vcd_obj;
  auto vcd_data = vcd_obj.parse_file(vcd_reference_file_path.string());

  ducode::DesignInstance instance = ducode::DesignInstance::create_instance(design);

  //instance.write_graphviz("ift_test_fft_apprx.dot");

  auto top_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) {
    return mod.get_name() == design.get_top_module_name();
  });
  REQUIRE(top_module != design.get_modules().end());

  auto timesteps_per_simulation_run = tag_size / top_module->get_input_names().size();
  timesteps_per_simulation_run = std::max<size_t>(timesteps_per_simulation_run, 1);
  auto number_simulation_runs = ducode::get_number_of_simulation_runs(stepsize, timesteps_per_simulation_run, vcd_data);
  //GF remove: auto number_simulation_runs = (vcd_data->get_timestamps().back() / stepsize) / gsl::narrow<double>(
  //                                                                                   timesteps_per_simulation_run);
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

TEST_CASE("ift_test_fft_eta2", "[ift_test][.]") {
  nlohmann::json params;
  params["ift"] = true;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;

  const int stepsize = 500;
  auto json_file = std::filesystem::path(TESTFILES_DIR) / "fft_eta2" / "fft16_eta2.json";
  auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / "fft_eta2" / "fft16_eta2.vcd";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  ducode::Design design = ducode::Design::parse_json(json_file);
  VCDFileParser vcd_obj;
  auto vcd_data = vcd_obj.parse_file(vcd_reference_file_path.string());

  auto exported_verilog = temp_dir / "ift_design.v";
  auto exported_testbench = temp_dir / "ift_testbench.v";
  auto exported_tags = temp_dir / "vcd_input.txt";

  design.write_verilog(exported_verilog, params);

  auto test_graph = design.get_hierarchy_graph();
  auto file = std::ofstream("fft_hierarchy_graph.dot", std::ofstream::out);
  //boost::write_graphviz(file, test_graph,
  //boost::make_label_writer(boost::get(boost::vertex_name_t(), test_graph)));
  std::unordered_map<TagInjectTargetSignal, TagTriggerConditionSignal> tagged_ports = {
      {TagInjectTargetSignal("tag_add1_1"), TagTriggerConditionSignal("err_add1_1")},
      {TagInjectTargetSignal("tag_add1_2"), TagTriggerConditionSignal("err_add1_2")},
      {TagInjectTargetSignal("tag_add2_1"), TagTriggerConditionSignal("err_add2_1")},
      {TagInjectTargetSignal("tag_add2_2"), TagTriggerConditionSignal("err_add2_2")},
      {TagInjectTargetSignal("tag_sub1_1"), TagTriggerConditionSignal("err_sub1_1")},
      {TagInjectTargetSignal("tag_sub1_2"), TagTriggerConditionSignal("err_sub1_2")},
      {TagInjectTargetSignal("tag_sub2_1"), TagTriggerConditionSignal("err_sub2_1")},
      {TagInjectTargetSignal("tag_sub2_2"), TagTriggerConditionSignal("err_sub2_2")}};

  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, &ducode::find_root_scope(vcd_data, design));
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));
  auto instance = ducode::DesignInstance::create_instance(design);
  testbench.add_tags(tagged_ports);
  testbench.write_verilog(exported_testbench, params);
  auto data = ducode::simulate_design(exported_verilog, exported_testbench);
  const VCDScope* vcd_root_scope = &ducode::find_root_scope(data, design);
  ducode::VCDSignalsDataManager test_value_map(data, vcd_root_scope);
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

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
