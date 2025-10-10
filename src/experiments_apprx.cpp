/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/instantiation_graph.hpp>
#include <ducode/instantiation_graph_traits.hpp>
#include <ducode/module.hpp>
#include <ducode/signals_data_manager.hpp>
#include <ducode/tag_generator.hpp>
#include <ducode/testbench.hpp>
#include <ducode/utility/VCD_utility.hpp>
#include <ducode/utility/concepts.hpp>
#include <ducode/utility/simulation.hpp>
#include <ducode/utility/types.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/detail/adjacency_list.hpp>
#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
#include <fmt/core.h>
#include <gsl/narrow>
#include <nlohmann/json_fwd.hpp>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <vcd-parser/VCDFile.hpp>
#include <vcd-parser/VCDFileParser.hpp>
#include <vcd-parser/VCDTypes.hpp>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <ranges>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

/*
 * Contains all experiments for the approximate design analysis paper
 */

namespace {
template<filesystem_like T>
ducode::DesignInstance run_multi_tag_batch_simulation(const ducode::Design& design,
                                                      const ducode::Module& top_module,
                                                      std::shared_ptr<VCDFile>& vcd_data,
                                                      const T& exported_verilog, const T& exported_testbench,
                                                      nlohmann::json& params) {
  const uint32_t tag_size = params.value("tag_size", 512);
  const ducode::Stepsize stepsize(params.value("stepsize", 1u));
  const auto timesteps_per_simulation_run = ducode::get_timesteps_per_simulation_run(top_module, tag_size);
  params["timesteps_per_simulation_run"] = timesteps_per_simulation_run;
  const auto number_simulation_runs = ducode::get_number_of_simulation_runs(stepsize, timesteps_per_simulation_run,
                                                                            vcd_data);
  const VCDScope* root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager tag_tracker(vcd_data, root_scope);
  //exporting testbench with IFT
  ducode::Testbench testbench(design, std::make_shared<ducode::VCDSignalsDataManager>(tag_tracker));
  testbench.set_tag_generator(
      std::make_unique<ducode::FullResolutionTagGenerator>(top_module.get_input_ports(), vcd_data));
  testbench.write_verilog(exported_testbench, params);

  auto exported_tags_files = ducode::get_exported_tags_files(number_simulation_runs);
  auto tags_times = ducode::get_tags_times(vcd_data, stepsize.value());
  auto exported_tags_vec = ducode::get_exported_tags_vec(exported_tags_files, number_simulation_runs,
                                                         tags_times, timesteps_per_simulation_run);
  testbench.write_tags(exported_tags_vec, params);

  auto instance = ducode::DesignInstance::create_instance(design);

  constexpr uint32_t sim_max_duration = 60;
  const auto max_time = std::chrono::system_clock::now() + std::chrono::minutes(sim_max_duration);

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
  ducode::VCDSignalsDataManager sim_data(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_shared<ducode::VCDSignalsDataManager>(sim_data));

  return instance;
}

template<filesystem_like T>
ducode::DesignInstance run_multi_tag_batch_simulation_random(const ducode::Design& design,
                                                             const ducode::Module& top_module,
                                                             std::shared_ptr<VCDFile>& vcd_data,
                                                             const T& exported_verilog, const T& exported_testbench,
                                                             uint32_t coverage,
                                                             nlohmann::json& params) {
  const uint32_t tag_size = params.value("tag_size", 512);
  const uint32_t stepsize_default = 500;
  ducode::Stepsize stepsize(params.value("stepsize", stepsize_default));
  const auto timesteps_per_simulation_run = ducode::get_timesteps_per_simulation_run(top_module, tag_size);
  params["timesteps_per_simulation_run"] = timesteps_per_simulation_run;
  const auto number_simulation_runs = ducode::get_number_of_simulation_runs(stepsize, timesteps_per_simulation_run,
                                                                            vcd_data);
  const VCDScope* root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager tag_tracker(vcd_data, root_scope);
  //exporting testbench with IFT
  ducode::Testbench testbench(design, std::make_shared<ducode::VCDSignalsDataManager>(tag_tracker));
  testbench.set_tag_generator(
      std::make_unique<ducode::DeterministicRandomTagGenerator>(top_module.get_input_ports(), vcd_data, coverage));
  testbench.write_verilog(exported_testbench, params);

  auto exported_tags_files = ducode::get_exported_tags_files(number_simulation_runs);
  auto tags_times = ducode::get_tags_times(vcd_data, gsl::narrow<uint32_t>(stepsize.value()));
  auto exported_tags_vec = ducode::get_exported_tags_vec(exported_tags_files, number_simulation_runs,
                                                         tags_times, timesteps_per_simulation_run);
  testbench.write_tags(exported_tags_vec, params);

  auto instance = ducode::DesignInstance::create_instance(design);

  constexpr uint32_t sim_max_duration = 60;
  const auto max_time = std::chrono::system_clock::now() + std::chrono::minutes(sim_max_duration);

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
  ducode::VCDSignalsDataManager sim_data(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_shared<ducode::VCDSignalsDataManager>(sim_data));

  return instance;
}

void write_heat_map(std::string tex_name,
                    std::unordered_map<std::string, std::unordered_map<std::string, uint32_t>> tag_cnt_map,
                    std::vector<std::string>& i_names, std::vector<std::string> o_names) {
  double largest_number = 0;
  for (auto& tag_cnt: tag_cnt_map) {
    for (auto& tags: tag_cnt.second) { largest_number = std::max<double>(tags.second, largest_number); }
  }

  std::ofstream ofv_tb(tex_name);
  if (!ofv_tb.is_open()) { throw std::runtime_error(fmt::format("Could not open file: {}", tex_name)); }
  ofv_tb << "\\begin{table*}\n";
  ofv_tb << "\\caption{Appx. Adder heat map}\n";
  ofv_tb << "\\label{tab:heat_map_random_rel}\n";
  ofv_tb << "\\begin{tabularx}{\\textwidth}{Xcccccccccc}\n";
  ofv_tb << "\\toprule\n";
  ofv_tb << "  & ";
  for (auto& name: o_names) {
    std::string nam = name;
    if (name.find("_") != std::string::npos) {
      const int num1 = 2;
      const int num2 = 8;
      nam.replace(num1, 1, "\\_");
      nam.replace(num2, 1, "\\_");
    }
    ofv_tb << nam;
    if (name != o_names.back()) {
      ofv_tb << "&";
    } else {
      ofv_tb << "  \\\\ \n";
    }
  }
  ofv_tb << "\\midrule\n";
  for (auto& name: i_names) {
    std::string nam = name;
    if (name.find("_") != std::string::npos) {
      const int num1 = 2;
      const int num2 = 8;
      nam.replace(num1, 1, "\\_");
      nam.replace(num2, 1, "\\_");
    }
    ofv_tb << nam << "&";
    std::vector<uint32_t> color_gradient = {0, 0, 0};
    for (auto& o_name: o_names) {
      const double num1 = 0.25;
      const double num2 = 0.50;
      const double num3 = 0.75;
      if (tag_cnt_map[o_name][name] == 0) {
        ofv_tb << "\\cellcolor{white}";
      } else if ((tag_cnt_map[o_name][name] / largest_number) <= num1) {
        ofv_tb << "\\cellcolor{green}";
      } else if ((tag_cnt_map[o_name][name] / largest_number) > num1 && (tag_cnt_map[o_name][name] / largest_number) <=
                                                                            num2) {
        ofv_tb << "\\cellcolor{yellow}";
      } else if ((tag_cnt_map[o_name][name] / largest_number) > num2 && (tag_cnt_map[o_name][name] / largest_number) <=
                                                                            num3) {
        ofv_tb << "\\cellcolor{orange}";
      }
      if ((tag_cnt_map[o_name][name] / largest_number) > num3) { ofv_tb << "\\cellcolor{red}"; }
      //      if (tag_cnt_map[o_name][name] == 0) {
      //        ofv_tb << "\\cellcolor[RGB]{255,255,255}";
      //      } else {
      //        double t = (tag_cnt_map[o_name][name] / largest_number);
      //        color_gradient[0] = (1-t) * 255;
      //        color_gradient[1] = (1-t) * 255;
      //        color_gradient[2] = t * 255;
      //        color_gradient[0] = 255 * (0.5 * sin(3.14 * (t + 0.5)) + 0.5);
      //        color_gradient[1] = 255 * sin(1.57 * t);
      //        color_gradient[2] = 255 * cos(1.57 * t);

      //        ofv_tb << fmt::format("\\cellcolor[RGB]{{ {},{},{} }}", color_gradient[0], color_gradient[1], color_gradient[2]);
      //      }
      ofv_tb << tag_cnt_map[o_name][name];
      if (o_name != o_names.back()) { ofv_tb << "&"; }
    }
    ofv_tb << "\\\\ \n";
  }

  ofv_tb << "\\end{tabularx}\n";
  ofv_tb << "\\end{table*}\n";


  ofv_tb.close();
};
}// namespace

TEST_CASE("appx_add", "[exper_apprx][.]") {
  nlohmann::json params;
  params["ift"] = true;
  params["apprx"] = true;

  const int stepsize = 500;
  params["stepsize"] = stepsize;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;
  auto json_file = std::filesystem::path(TESTFILES_DIR) / "fft_appx_add" / "fft16_apx.json";
  auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / "fft_appx_add" / "tb_fft16_ofdm_apx.vcd";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  ducode::Design design = ducode::Design::parse_json(json_file);
  VCDFileParser vcd_obj;
  auto vcd_data = vcd_obj.parse_file(vcd_reference_file_path.string());

  auto exported_verilog = temp_dir / ("ift_design.v");
  auto exported_testbench = temp_dir / "ift_testbench.v";
  auto exported_tags = temp_dir / "vcd_input.txt";

  auto tb_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });
  const uint32_t coverage = 50;

  design.write_verilog(exported_verilog, params);

  ducode::DesignInstance instance = run_multi_tag_batch_simulation_random(
      design, *tb_module, vcd_data, exported_verilog,
      exported_testbench, coverage, params);

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

TEST_CASE("appx_add_experiments", "[exper_apprx][.]") {
  nlohmann::json params;
  params["ift"] = true;
  params["apprx"] = true;
  nlohmann::json result_data;

  const int stepsize = 500;
  params["stepsize"] = stepsize;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;
  auto json_file = std::filesystem::path(TESTFILES_DIR) / "apprx_add_experiments" / "fft16_apx.json";
  //  auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / "appx_add_experiments" / "tb_fft16_ofdm_apx.vcd";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  ducode::Design design = ducode::Design::parse_json(json_file);

  auto exported_verilog = temp_dir / ("ift_design.v");
  auto exported_testbench = temp_dir / "ift_testbench.v";
  auto exported_tags = temp_dir / "vcd_input.txt";

  auto tb_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });

  design.write_verilog(exported_verilog, params);
  const int max_num = 6;
  for (uint32_t i = 0; i < max_num; i++) {
    std::string vcd_file = "tb_fft16_ofdm_apx_";
    vcd_file += std::to_string(i) + ".vcd";
    auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / "apprx_add_experiments" / vcd_file;
    VCDFileParser vcd_obj;
    auto vcd_data = vcd_obj.parse_file(vcd_reference_file_path.string());

    const uint32_t coverage = 50;
    auto simulation_runtime_start = std::chrono::high_resolution_clock::now();

    ducode::DesignInstance instance = run_multi_tag_batch_simulation_random(
        design, *tb_module, vcd_data, exported_verilog,
        exported_testbench, coverage, params);

    auto simulation_runtime_end = std::chrono::high_resolution_clock::now();
    result_data["runtime_" + vcd_file] = std::chrono::duration_cast<std::chrono::milliseconds>(
                                             simulation_runtime_end - simulation_runtime_start)
                                             .count();

    std::set<uint32_t> tags;
    for (auto [edge_it, edge_end] = boost::edges(instance.m_graph); edge_it != edge_end; ++edge_it) {
      if (instance.m_graph[*edge_it].net_ptr != nullptr && (instance.m_graph[*edge_it].net_ptr->get_name() == "qi" || instance.m_graph[*edge_it].net_ptr->get_name() ==
                                                                                                                          "qr")) {
        auto tag_values = instance.get_tag_values(*edge_it);
        for (const auto& tag_vector: tag_values) {
          for (const auto& tag_value: tag_vector.m_timed_signal_values) {
            auto value_vector = tag_value.value.get_value_vector();
            for (const auto& [index, tag_bit]: std::views::enumerate(value_vector)) {
              if (tag_bit == ducode::SignalBit::BIT_1) {
                auto uindex = gsl::narrow<uint64_t>(index);
                if (!tags.contains(static_cast<unsigned int>(pow(
                        2, (static_cast<double>(tag_value.value.get_value_vector().size() - uindex - 1)))))) {
                  tags.insert(static_cast<unsigned int>(pow(
                      2, (static_cast<double>(tag_value.value.get_value_vector().size() - uindex - 1)))));
                }
              }
            }
          }
        }
      }
    }
    result_data["injected_tags_@_outputs" + vcd_file] = tags.size();// remove the 0 entry; not a tag
  }
  std::fstream os_json("results.json", std::ios::out);
  os_json << result_data;
  os_json.close();
}

////// TEST CASES FOR THE EXPERIMENTS FOR THE PAPER "Detailed Insight into Approximate Circuits with Error Responsive Information Flow Tracking"
TEST_CASE("fft16_full_res", "[exper_exact][.]") {
  nlohmann::json params;
  params["ift"] = true;
  params["apprx"] = false;
  params["apprx_mult"] = false;
  nlohmann::json result_data;

  const int stepsize = 500;
  params["stepsize"] = stepsize;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;
  auto json_file = std::filesystem::path(TESTFILES_DIR) / "fft16" / "fft16.json";
  auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / "fft16" / "tb_fft16_v2.vcd";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  spdlog::debug("Temporary directory created at: {}", temp_dir.string());
  ducode::Design design = ducode::Design::parse_json(json_file);
  VCDFileParser vcd_obj;
  auto vcd_data = vcd_obj.parse_file(vcd_reference_file_path.string());

  auto exported_verilog = temp_dir / ("ift_design.v");
  auto exported_testbench = temp_dir / "ift_testbench.v";
  auto exported_tags = temp_dir / "vcd_input.txt";

  auto tb_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });

  design.write_verilog(exported_verilog, params);

  std::string vcd_file = "tb_fft16_ofdm_";
  std::string heat_map_file = vcd_file + "_hmap_full_res";
  vcd_file += std::to_string(0) + ".vcd";

  auto simulation_runtime_start = std::chrono::high_resolution_clock::now();

  ducode::DesignInstance instance = run_multi_tag_batch_simulation(
      design, *tb_module, vcd_data, exported_verilog,
      exported_testbench, params);

  auto simulation_runtime_end = std::chrono::high_resolution_clock::now();
  result_data["runtime_" + vcd_file] = std::chrono::duration_cast<std::chrono::milliseconds>(
                                           simulation_runtime_end - simulation_runtime_start)
                                           .count();

  uint32_t tag_cnt_outputs_total = 0;
  uint32_t tag_cnt_outputs = 0;
  uint32_t tag_cnt_ci_inputs = 0;
  std::unordered_map<std::string, uint32_t> tag_cnt_inputs;
  std::unordered_map<std::string, uint32_t> tag_cnt_output_1_input;
  std::unordered_map<std::string, std::unordered_map<std::string, uint32_t>> tag_cnt_outputs_rel;
  std::vector<std::string> inputs_names = {"clk", "rstn", "ena", "xr", "xi"};
  std::vector<std::string> outputs_names = {"qr", "qi"};

  tag_cnt_outputs_total = instance.tag_cnt_multiple_edges(outputs_names);

  for (auto& name: inputs_names) { tag_cnt_inputs[name] = instance.tag_cnt_single_edge(name); }
  for (auto& o_name: outputs_names) {
    tag_cnt_output_1_input.clear();
    for (auto& i_name: inputs_names) { tag_cnt_output_1_input[i_name] = instance.unique_tag_cnt_from_input_to_single_output(i_name, o_name); }
    tag_cnt_outputs_rel[o_name] = tag_cnt_output_1_input;
  }

  write_heat_map((heat_map_file + std::to_string(0) + "_heat_map.tex"), tag_cnt_outputs_rel, inputs_names,
                 outputs_names);

  result_data["injected_tags_@_outputs__total" + vcd_file] = tag_cnt_outputs_total;
  std::fstream os_json("results_fullres.json", std::ios::out);
  os_json << result_data;
  os_json.close();
}

TEST_CASE("fft16_random_res", "[exper_exact][.]") {
  nlohmann::json params;
  params["ift"] = true;
  params["apprx"] = false;
  params["apprx_mult"] = false;
  nlohmann::json result_data;

  const int stepsize = 500;
  params["stepsize"] = stepsize;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;
  auto json_file = std::filesystem::path(TESTFILES_DIR) / "fft16" / "fft16.json";
  auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / "fft16" / "tb_fft16_v2.vcd";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  spdlog::debug("Temporary directory created at: {}", temp_dir.string());
  ducode::Design design = ducode::Design::parse_json(json_file);
  VCDFileParser vcd_obj;
  auto vcd_data = vcd_obj.parse_file(vcd_reference_file_path.string());

  auto exported_verilog = temp_dir / ("ift_design.v");
  auto exported_testbench = temp_dir / "ift_testbench.v";
  auto exported_tags = temp_dir / "vcd_input.txt";

  auto tb_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });
  const int tag_number = 500;
  const uint32_t coverage = 50;// need to figure out how much coverage 500 tags was before...
  design.write_verilog(exported_verilog, params);

  std::string vcd_file = "tb_fft16_ofdm";
  std::string heat_map_file = vcd_file + "_random_res";
  vcd_file += std::to_string(0) + ".vcd";

  auto simulation_runtime_start = std::chrono::high_resolution_clock::now();

  ducode::DesignInstance instance = run_multi_tag_batch_simulation_random(
      design, *tb_module, vcd_data, exported_verilog,
      exported_testbench, coverage, params);

  auto simulation_runtime_end = std::chrono::high_resolution_clock::now();
  result_data["runtime_" + vcd_file] = std::chrono::duration_cast<std::chrono::milliseconds>(
                                           simulation_runtime_end - simulation_runtime_start)
                                           .count();

  uint32_t tag_cnt_outputs_total = 0;
  uint32_t tag_cnt_outputs = 0;
  uint32_t tag_cnt_ci_inputs = 0;
  std::unordered_map<std::string, uint32_t> tag_cnt_inputs;
  std::unordered_map<std::string, uint32_t> tag_cnt_output_1_input;
  std::unordered_map<std::string, std::unordered_map<std::string, uint32_t>> tag_cnt_outputs_rel;
  std::vector<std::string> inputs_names = {"clk", "rstn", "ena", "xr", "xi"};
  std::vector<std::string> outputs_names = {"qr", "qi"};

  tag_cnt_outputs_total = instance.tag_cnt_multiple_edges(outputs_names);

  for (auto& name: inputs_names) { tag_cnt_inputs[name] = instance.tag_cnt_single_edge(name); }
  for (auto& o_name: outputs_names) {
    tag_cnt_output_1_input.clear();
    for (auto& i_name: inputs_names) { tag_cnt_output_1_input[i_name] = instance.unique_tag_cnt_from_input_to_single_output(i_name, o_name); }
    tag_cnt_outputs_rel[o_name] = tag_cnt_output_1_input;
  }

  write_heat_map((heat_map_file + std::to_string(0) + "_heat_map.tex"), tag_cnt_outputs_rel, inputs_names,
                 outputs_names);

  result_data["injected_tags_@_outputs__total" + vcd_file] = tag_cnt_outputs_total;
  std::fstream os_json("results_randomres.json", std::ios::out);
  os_json << result_data;
  os_json.close();
}

TEST_CASE("appx_add_experiments_larger_WI", "[exper_apprx_wi][.]") {
  nlohmann::json params;
  params["ift"] = true;
  params["apprx"] = true;
  nlohmann::json result_data;

  const int stepsize = 500;
  params["stepsize"] = stepsize;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;
  auto json_file = std::filesystem::path(TESTFILES_DIR) / "apprx_add_experiments_WI" / "fft16_apx.json";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  ducode::Design design = ducode::Design::parse_json(json_file);

  auto exported_verilog = temp_dir / ("ift_design.v");
  auto exported_testbench = temp_dir / "ift_testbench.v";
  auto exported_tags = temp_dir / "vcd_input.txt";

  design.write_verilog(exported_verilog, params);
  for (uint32_t i = 3; i < 4; i++) {
    std::string vcd_file = "tb_fft16_ofdm_apx_";
    std::string heat_map_file = vcd_file + "_hmap_random_res";

    vcd_file += std::to_string(i) + ".vcd";
    auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / "apprx_add_experiments_WI" / vcd_file;
    VCDFileParser vcd_obj;
    auto vcd_data = vcd_obj.parse_file(vcd_reference_file_path.string());
    const int tag_number = 0;

    auto simulation_runtime_start = std::chrono::high_resolution_clock::now();

    std::unordered_map<TagInjectTargetSignal, TagTriggerConditionSignal> tagged_ports = {
        {TagInjectTargetSignal("ci_add1_1"), TagTriggerConditionSignal("co_add1_1")},
        {TagInjectTargetSignal("ci_add1_2"), TagTriggerConditionSignal("co_add1_2")},
        {TagInjectTargetSignal("ci_add2_1"), TagTriggerConditionSignal("co_add2_1")},
        {TagInjectTargetSignal("ci_add2_2"), TagTriggerConditionSignal("co_add2_2")},
        {TagInjectTargetSignal("ci_sub1_1"), TagTriggerConditionSignal("co_sub1_1")},
        {TagInjectTargetSignal("ci_sub1_2"), TagTriggerConditionSignal("co_sub1_2")},
        {TagInjectTargetSignal("ci_sub2_1"), TagTriggerConditionSignal("co_sub2_1")},
        {TagInjectTargetSignal("ci_sub2_2"), TagTriggerConditionSignal("co_sub2_2")}};

    const VCDScope* root_scope = &ducode::find_root_scope(vcd_data, design);
    ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
    ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));

    testbench.add_tags(tagged_ports);
    testbench.write_verilog(exported_testbench, params);
    auto instance = ducode::DesignInstance::create_instance(design);

    std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;
    for (const auto& exported_tags: ducode::get_exported_tags_files(ducode::get_number_of_simulation_runs(gsl::narrow<uint32_t>(ducode::get_stepsize(vcd_data, "clk", "tb_fft16_ofdm_apx")), ducode::get_timesteps_per_simulation_run(design.get_top_module(), tag_size), vcd_data))) {
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

    auto simulation_runtime_end = std::chrono::high_resolution_clock::now();
    result_data["runtime_" + vcd_file] = std::chrono::duration_cast<std::chrono::milliseconds>(
                                             simulation_runtime_end - simulation_runtime_start)
                                             .count();
    uint32_t tag_cnt_outputs_total = 0;
    uint32_t tag_cnt_outputs = 0;
    uint32_t tag_cnt_ci_inputs = 0;
    std::unordered_map<std::string, uint32_t> tag_cnt_inputs;
    std::unordered_map<std::string, uint32_t> tag_cnt_output_1_input;
    std::unordered_map<std::string, std::unordered_map<std::string, uint32_t>> tag_cnt_outputs_rel;
    std::vector<std::string> inputs_names = {
        "ci_add1_1", "ci_add1_2", "ci_sub1_1", "ci_sub1_2", "ci_add2_1", "ci_add2_2", "ci_sub2_1", "ci_sub2_2", "clk",
        "rstn", "ena", "xr", "xi"};
    std::vector<std::string> outputs_names = {
        "co_add1_1", "co_add1_2", "co_sub1_1", "co_sub1_2", "co_add2_1", "co_add2_2", "co_sub2_1", "co_sub2_2", "qr", "qi"};
    std::vector<std::string> ci_outputs_names = {
        "co_add1_1", "co_add1_2", "co_sub1_1", "co_sub1_2", "co_add2_1", "co_add2_2", "co_sub2_1", "co_sub2_2"};

    tag_cnt_outputs_total = instance.tag_cnt_multiple_edges(outputs_names);
    tag_cnt_outputs = instance.tag_cnt_multiple_edges(ci_outputs_names);
    const int index0 = 0;
    const int index1 = 1;
    const int index2 = 2;
    const int index3 = 3;
    const int index4 = 4;
    const int index5 = 5;
    const int index6 = 6;
    const int index7 = 7;
    tag_cnt_ci_inputs += instance.tag_cnt_single_edge(inputs_names[index0]);
    tag_cnt_ci_inputs += instance.tag_cnt_single_edge(inputs_names[index1]);
    tag_cnt_ci_inputs += instance.tag_cnt_single_edge(inputs_names[index2]);
    tag_cnt_ci_inputs += instance.tag_cnt_single_edge(inputs_names[index3]);
    tag_cnt_ci_inputs += instance.tag_cnt_single_edge(inputs_names[index4]);
    tag_cnt_ci_inputs += instance.tag_cnt_single_edge(inputs_names[index5]);
    tag_cnt_ci_inputs += instance.tag_cnt_single_edge(inputs_names[index6]);
    tag_cnt_ci_inputs += instance.tag_cnt_single_edge(inputs_names[index7]);
    for (auto& name: inputs_names) { tag_cnt_inputs[name] = instance.tag_cnt_single_edge(name); }
    for (auto& o_name: outputs_names) {
      tag_cnt_output_1_input.clear();
      for (auto& i_name: inputs_names) { tag_cnt_output_1_input[i_name] = instance.unique_tag_cnt_from_input_to_single_output(i_name, o_name); }
      tag_cnt_outputs_rel[o_name] = tag_cnt_output_1_input;
    }

    std::vector<std::string> inputs_names_heat_map = {
        "ci_add1_1", "ci_add1_2", "ci_sub1_1", "ci_sub1_2", "ci_add2_1", "ci_add2_2", "ci_sub2_1", "ci_sub2_2"};

    write_heat_map((heat_map_file + std::to_string(i) + "_heat_map.tex"), tag_cnt_outputs_rel, inputs_names_heat_map,
                   outputs_names);

    result_data["injected_tags_@_ci_outputs_" + vcd_file] = tag_cnt_outputs;
    result_data["injected_tags_@_ci_inputs_" + vcd_file] = tag_cnt_ci_inputs;
    result_data["injected_tags_@_outputs__total" + vcd_file] = tag_cnt_outputs_total;
  }
  std::fstream os_json("results.json", std::ios::out);
  os_json << result_data;
  os_json.close();
}

TEST_CASE("appx_add_experiments_full_res", "[exper_apprx_wi][.]") {
  nlohmann::json params;
  params["ift"] = true;
  params["apprx"] = false;
  nlohmann::json result_data;

  const int stepsize = 500;
  params["stepsize"] = stepsize;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;
  auto json_file = std::filesystem::path(TESTFILES_DIR) / "apprx_add_experiments_WI" / "fft16_apx.json";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  ducode::Design design = ducode::Design::parse_json(json_file);

  auto exported_verilog = temp_dir / ("ift_design.v");
  auto exported_testbench = temp_dir / "ift_testbench.v";
  auto exported_tags = temp_dir / "vcd_input.txt";

  auto tb_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });

  design.write_verilog(exported_verilog, params);
  for (uint32_t i = 3; i < 4; i++) {
    std::string vcd_file = "tb_fft16_ofdm_apx_";
    std::string heat_map_file = vcd_file + "_hmap_full_res";
    vcd_file += std::to_string(i) + ".vcd";
    auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / "apprx_add_experiments" / vcd_file;
    VCDFileParser vcd_obj;
    auto vcd_data = vcd_obj.parse_file(vcd_reference_file_path.string());

    auto simulation_runtime_start = std::chrono::high_resolution_clock::now();

    ducode::DesignInstance instance = run_multi_tag_batch_simulation(
        design, *tb_module, vcd_data, exported_verilog,
        exported_testbench, params);

    auto simulation_runtime_end = std::chrono::high_resolution_clock::now();
    result_data["runtime_" + vcd_file] = std::chrono::duration_cast<std::chrono::milliseconds>(
                                             simulation_runtime_end - simulation_runtime_start)
                                             .count();

    uint32_t tag_cnt_outputs_total = 0;
    uint32_t tag_cnt_outputs = 0;
    uint32_t tag_cnt_ci_inputs = 0;
    std::unordered_map<std::string, uint32_t> tag_cnt_inputs;
    std::unordered_map<std::string, uint32_t> tag_cnt_output_1_input;
    std::unordered_map<std::string, std::unordered_map<std::string, uint32_t>> tag_cnt_outputs_rel;
    std::vector<std::string> inputs_names = {
        "ci_add1_1", "ci_add1_2", "ci_sub1_1", "ci_sub1_2", "ci_add2_1", "ci_add2_2", "ci_sub2_1", "ci_sub2_2", "clk",
        "rstn", "ena", "xr", "xi"};
    std::vector<std::string> outputs_names = {
        "co_add1_1", "co_add1_2", "co_sub1_1", "co_sub1_2", "co_add2_1", "co_add2_2", "co_sub2_1", "co_sub2_2", "qr", "qi"};
    std::vector<std::string> ci_outputs_names = {
        "co_add1_1", "co_add1_2", "co_sub1_1", "co_sub1_2", "co_add2_1", "co_add2_2", "co_sub2_1", "co_sub2_2"};

    tag_cnt_outputs_total = instance.tag_cnt_multiple_edges(outputs_names);
    tag_cnt_outputs = instance.tag_cnt_multiple_edges(ci_outputs_names);
    const int index0 = 0;
    const int index1 = 1;
    const int index2 = 2;
    const int index3 = 3;
    const int index4 = 4;
    const int index5 = 5;
    const int index6 = 6;
    const int index7 = 7;
    tag_cnt_ci_inputs += instance.tag_cnt_single_edge(inputs_names[index0]);
    tag_cnt_ci_inputs += instance.tag_cnt_single_edge(inputs_names[index1]);
    tag_cnt_ci_inputs += instance.tag_cnt_single_edge(inputs_names[index2]);
    tag_cnt_ci_inputs += instance.tag_cnt_single_edge(inputs_names[index3]);
    tag_cnt_ci_inputs += instance.tag_cnt_single_edge(inputs_names[index4]);
    tag_cnt_ci_inputs += instance.tag_cnt_single_edge(inputs_names[index5]);
    tag_cnt_ci_inputs += instance.tag_cnt_single_edge(inputs_names[index6]);
    tag_cnt_ci_inputs += instance.tag_cnt_single_edge(inputs_names[index7]);
    for (auto& name: inputs_names) { tag_cnt_inputs[name] = instance.tag_cnt_single_edge(name); }
    for (auto& o_name: outputs_names) {
      tag_cnt_output_1_input.clear();
      for (auto& i_name: inputs_names) { tag_cnt_output_1_input[i_name] = instance.unique_tag_cnt_from_input_to_single_output(i_name, o_name); }
      tag_cnt_outputs_rel[o_name] = tag_cnt_output_1_input;
    }

    write_heat_map((heat_map_file + std::to_string(i) + "_heat_map.tex"), tag_cnt_outputs_rel, inputs_names,
                   outputs_names);

    result_data["injected_tags_@_ci_outputs_" + vcd_file] = tag_cnt_outputs;
    result_data["injected_tags_@_ci_inputs_" + vcd_file] = tag_cnt_ci_inputs;
    result_data["injected_tags_@_outputs__total" + vcd_file] = tag_cnt_outputs_total;
  }
  std::fstream os_json("results_fullres.json", std::ios::out);
  os_json << result_data;
  os_json.close();
}

TEST_CASE("appx_add_experiments_randoml_res", "[exper_apprx_wi][.]") {
  nlohmann::json params;
  params["ift"] = true;
  params["apprx"] = false;
  nlohmann::json result_data;

  const int stepsize = 500;
  params["stepsize"] = stepsize;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;
  auto json_file = std::filesystem::path(TESTFILES_DIR) / "apprx_add_experiments_WI" / "fft16_apx.json";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  ducode::Design design = ducode::Design::parse_json(json_file);

  auto exported_verilog = temp_dir / ("ift_design.v");
  auto exported_testbench = temp_dir / "ift_testbench.v";
  auto exported_tags = temp_dir / "vcd_input.txt";

  auto tb_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });

  design.write_verilog(exported_verilog, params);
  for (uint32_t i = 3; i < 4; i++) {
    std::string vcd_file = "tb_fft16_ofdm_apx_";
    std::string heat_map_file = vcd_file;
    vcd_file += std::to_string(i) + ".vcd";
    auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / "apprx_add_experiments" / vcd_file;
    VCDFileParser vcd_obj;
    auto vcd_data = vcd_obj.parse_file(vcd_reference_file_path.string());

    const int tag_number = 500;
    const uint32_t coverage = 50;// need to figure out how much coverage 500 tags was before...
    auto simulation_runtime_start = std::chrono::high_resolution_clock::now();

    ducode::DesignInstance instance = run_multi_tag_batch_simulation_random(
        design, *tb_module, vcd_data, exported_verilog,
        exported_testbench, coverage, params);

    auto simulation_runtime_end = std::chrono::high_resolution_clock::now();
    result_data["runtime_" + vcd_file] = std::chrono::duration_cast<std::chrono::milliseconds>(
                                             simulation_runtime_end - simulation_runtime_start)
                                             .count();

    uint32_t tag_cnt_outputs_total = 0;
    uint32_t tag_cnt_outputs = 0;
    uint32_t tag_cnt_ci_inputs = 0;
    std::unordered_map<std::string, uint32_t> tag_cnt_inputs;
    std::unordered_map<std::string, uint32_t> tag_cnt_output_1_input;
    std::unordered_map<std::string, std::unordered_map<std::string, uint32_t>> tag_cnt_outputs_rel;
    std::vector<std::string> inputs_names = {
        "ci_add1_1", "ci_add1_2", "ci_sub1_1", "ci_sub1_2", "ci_add2_1", "ci_add2_2", "ci_sub2_1", "ci_sub2_2", "clk",
        "rstn", "ena", "xr", "xi"};
    std::vector<std::string> outputs_names = {
        "co_add1_1", "co_add1_2", "co_sub1_1", "co_sub1_2", "co_add2_1", "co_add2_2", "co_sub2_1", "co_sub2_2", "qr", "qi"};
    std::vector<std::string> ci_outputs_names = {
        "co_add1_1", "co_add1_2", "co_sub1_1", "co_sub1_2", "co_add2_1", "co_add2_2", "co_sub2_1", "co_sub2_2"};

    tag_cnt_outputs_total = instance.tag_cnt_multiple_edges(outputs_names);
    tag_cnt_outputs = instance.tag_cnt_multiple_edges(ci_outputs_names);
    const int index0 = 0;
    const int index1 = 1;
    const int index2 = 2;
    const int index3 = 3;
    const int index4 = 4;
    const int index5 = 5;
    const int index6 = 6;
    const int index7 = 7;
    tag_cnt_ci_inputs += instance.tag_cnt_single_edge(inputs_names[index0]);
    tag_cnt_ci_inputs += instance.tag_cnt_single_edge(inputs_names[index1]);
    tag_cnt_ci_inputs += instance.tag_cnt_single_edge(inputs_names[index2]);
    tag_cnt_ci_inputs += instance.tag_cnt_single_edge(inputs_names[index3]);
    tag_cnt_ci_inputs += instance.tag_cnt_single_edge(inputs_names[index4]);
    tag_cnt_ci_inputs += instance.tag_cnt_single_edge(inputs_names[index5]);
    tag_cnt_ci_inputs += instance.tag_cnt_single_edge(inputs_names[index6]);
    tag_cnt_ci_inputs += instance.tag_cnt_single_edge(inputs_names[index7]);
    for (auto& name: inputs_names) { tag_cnt_inputs[name] = instance.tag_cnt_single_edge(name); }
    for (auto& o_name: outputs_names) {
      tag_cnt_output_1_input.clear();
      for (auto& i_name: inputs_names) { tag_cnt_output_1_input[i_name] = instance.unique_tag_cnt_from_input_to_single_output(i_name, o_name); }
      tag_cnt_outputs_rel[o_name] = tag_cnt_output_1_input;
    }

    write_heat_map((heat_map_file + std::to_string(i) + "_heat_map.tex"), tag_cnt_outputs_rel, inputs_names,
                   outputs_names);

    result_data["injected_tags_@_ci_outputs_" + vcd_file] = tag_cnt_outputs;
    result_data["injected_tags_@_ci_inputs_" + vcd_file] = tag_cnt_ci_inputs;
    result_data["injected_tags_@_outputs__total" + vcd_file] = tag_cnt_outputs_total;
  }
  std::fstream os_json("results_random.json", std::ios::out);
  os_json << result_data;
  os_json.close();
}

int main(int argc, char* argv[]) {
  // global setup...

  //making two sinks, one for consol, one for log file
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  console_sink->set_level(spdlog::level::info);
  console_sink->set_pattern("%^[%l] %s:%# %! %v");

  auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("tester.log", true);
  file_sink->set_level(spdlog::level::trace);
  file_sink->set_pattern("%^[%l] %s:%# %! %v");
  spdlog::sinks_init_list sink_list = {file_sink, console_sink};

  //registering the logger so it stays available for testcases
  spdlog::logger logger("duLog", sink_list.begin(), sink_list.end());
  logger.set_level(spdlog::level::trace);
  spdlog::register_logger(std::make_shared<spdlog::logger>(logger));

  //setting the registered logger to default
  auto log = spdlog::get("duLog");
  spdlog::set_default_logger(log);

  spdlog::debug("Configured LOGGER");

  int result = Catch::Session().run(argc, argv);

  spdlog::shutdown();

  return result;
}
