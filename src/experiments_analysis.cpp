/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/ift_analysis.hpp>
#include <ducode/instantiation_graph.hpp>
#include <ducode/signals_data_manager.hpp>
#include <ducode/tag_generator.hpp>
#include <ducode/testbench.hpp>
#include <ducode/utility/VCD_utility.hpp>
#include <ducode/utility/iverilog_wrapper.hpp>
#include <ducode/utility/simulation.hpp>

#include <boost/filesystem/operations.hpp>
#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
#include <gsl/assert>
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
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

/*
 * Contains all experiments for the IFT_Analysis paper
 */

namespace {
void run_experiment_full_resolution(const std::pair<std::string, std::string>& designs) {
  nlohmann::json params;
  auto start_overall = std::chrono::high_resolution_clock::now();

  nlohmann::json result_data_full;
  params["ift"] = true;
  const uint32_t tag_size = 32;// default tag size
  params["tag_size"] = tag_size;

  auto json_file = std::filesystem::path(TESTFILES_DIR) / designs.first / (designs.first + ".json");
  auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / designs.first / (designs.first + ".vcd");

  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  Expects(created);

  auto exported_verilog = temp_dir / ("ift" + designs.first + ".v");
  auto exported_testbench = temp_dir / ("ift" + designs.first + "_tb.v");

  ducode::Design design = ducode::Design::parse_json(json_file);

  VCDFileParser vcd_obj;
  spdlog::info("Loading Original Simulation Data");
  auto vcd_data = vcd_obj.parse_file(vcd_reference_file_path.string());

  //export the design with IFT
  design.write_verilog(exported_verilog, params);

  //get top module
  auto top_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });

  ducode::Stepsize stepsize(vcd_data, designs.second);
  const auto timesteps_per_simulation_run = ducode::get_timesteps_per_simulation_run(*top_module, tag_size);
  params["timesteps_per_simulation_run"] = timesteps_per_simulation_run;
  const auto number_simulation_runs = ducode::get_number_of_simulation_runs(stepsize, timesteps_per_simulation_run,
                                                                            vcd_data);

  //exporting testbench with IFT
  const VCDScope* root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));
  testbench.set_tag_generator(
      std::make_unique<ducode::FullResolutionTagGenerator>(top_module->get_input_ports(), vcd_data));
  testbench.write_verilog(exported_testbench, params);

  auto exported_tags_files = ducode::get_exported_tags_files(number_simulation_runs);
  auto tags_times = ducode::get_tags_times(vcd_data, gsl::narrow<uint32_t>(stepsize.value()));
  auto exported_tags_vec = ducode::get_exported_tags_vec(exported_tags_files, number_simulation_runs,
                                                         tags_times, timesteps_per_simulation_run);
  testbench.write_tags(exported_tags_vec, params);

  auto instance = ducode::DesignInstance::create_instance(design);

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

  //    auto stop_ift_simulation = std::chrono::high_resolution_clock::now();

  ///fill json object with relevant information about design -> then add it to result data json
  nlohmann::json ift_res = ducode::do_analysis(instance, design);
  const uint32_t number_of_injected_tags_total = static_cast<uint32_t>((vcd_data->get_timestamps().back() * top_module->get_input_names().size()) / ducode::get_stepsize(vcd_data, designs.second));
  const uint64_t number_of_simulated_tags = instance.m_signal_tag_map->get_vcd_data_size() * 32;

  if (number_of_injected_tags_total > number_of_simulated_tags) {
    ift_res["number_of_injected_tags"] = number_of_simulated_tags;
  } else {
    ift_res["number_of_injected_tags"] = number_of_injected_tags_total;
  }
  auto end_overall = std::chrono::high_resolution_clock::now();
  result_data_full["overall_runtime"] = std::chrono::duration_cast<std::chrono::seconds>(end_overall - start_overall).count();
  result_data_full[designs.first] = ift_res;
  instance.write_graphviz("exp_graph.dot");
  std::fstream os_json("results/results_full_" + designs.first + ".json", std::ios::out);
  os_json << result_data_full;
  os_json.close();
}
}// namespace

TEST_CASE("full_y86", "[exp_full_res][.]") {
  std::pair<std::string, std::string> designs = {"y86", "clk"};
  run_experiment_full_resolution(designs);
}

TEST_CASE("full_SPI_Master", "[exp_full_res][.]") {
  std::pair<std::string, std::string> designs = {"SPI_Master", "clk"};
  run_experiment_full_resolution(designs);
}

TEST_CASE("full_SPI_Slave", "[exp_full_res][.]") {
  std::pair<std::string, std::string> designs = {"SPI_Slave", "slaveSCLK"};
  run_experiment_full_resolution(designs);
}

TEST_CASE("full_yadmc", "[exp_full_res][.]") {
  std::pair<std::string, std::string> designs = {"yadmc", "sys_clk"};
  run_experiment_full_resolution(designs);
}

TEST_CASE("full_aew_128", "[exp_full_res][.]") {
  std::pair<std::string, std::string> designs = {"aes_128", "clk"};
  run_experiment_full_resolution(designs);
}

TEST_CASE("full_usb2uart", "[exp_full_res][.]") {
  std::pair<std::string, std::string> designs = {"usb2uart", "sys_clk"};
  run_experiment_full_resolution(designs);
}

TEST_CASE("full_openMSP430", "[exp_full_res][.]") {
  std::pair<std::string, std::string> designs = {"openMSP430", "mclk"};
  run_experiment_full_resolution(designs);
}

// TEST_CASE("full_mips_16_core_top", "[exp_full_res][.]") {
//   std::pair<std::string, std::string> designs = {"mips_16_core_top", "clk"};
//   run_experiment_full_resolution(designs);
// }

namespace {
void run_experiment_random_resolution(const std::pair<std::string, std::string>& designs) {
  nlohmann::json params;
  //  auto start_overall = std::chrono::high_resolution_clock::now();

  std::map<uint32_t, std::vector<std::pair<std::string, uint32_t>>> const tag_count_map;
  nlohmann::json result_data_random;

  params["ift"] = false;
  const uint32_t tag_size = 32;// default tag size
  params["tag_size"] = tag_size;

  auto json_file = std::filesystem::path(TESTFILES_DIR) / designs.first / (designs.first + ".json");
  auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / designs.first / (designs.first + ".vcd");

  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  auto exported_verilog = temp_dir / ("ift" + designs.first + ".v");
  auto exported_testbench = temp_dir / ("ift" + designs.first + "_tb.v");

  ducode::Design design = ducode::Design::parse_json(json_file);
  //export the design with IFT
  design.write_verilog(exported_verilog, params);

  VCDFileParser vcd_obj;
  spdlog::info("Loading Original Simulation Data");
  auto vcd_data = vcd_obj.parse_file(vcd_reference_file_path.string());

  //get top module
  auto top_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });

  params["ift"] = true;

  design.write_verilog(exported_verilog, params);

  ducode::Stepsize stepsize(vcd_data, designs.second);
  const auto timesteps_per_simulation_run = ducode::get_timesteps_per_simulation_run(*top_module, tag_size);
  params["timesteps_per_simulation_run"] = timesteps_per_simulation_run;
  const auto number_simulation_runs = ducode::get_number_of_simulation_runs(stepsize, timesteps_per_simulation_run,
                                                                            vcd_data);
  //exporting testbench with IFT
  const VCDScope* root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));

  const std::vector<uint32_t> coverage_percentages = {25, 50, 75, 100};
  for (const auto& coverage: coverage_percentages) {
    auto simulation_start = std::chrono::high_resolution_clock::now();
    std::map<std::string, std::vector<uint32_t>> const tag_counts;

    testbench.set_tag_generator(
        std::make_unique<ducode::DeterministicRandomTagGenerator>(top_module->get_input_ports(), vcd_data, coverage));
    testbench.write_verilog(exported_testbench, params);

    auto exported_tags_files = ducode::get_exported_tags_files(number_simulation_runs);
    auto tags_times = ducode::get_tags_times(vcd_data, gsl::narrow<uint32_t>(stepsize.value()));
    auto exported_tags_vec = ducode::get_exported_tags_vec(exported_tags_files, number_simulation_runs,
                                                           tags_times, timesteps_per_simulation_run);
    testbench.write_tags(exported_tags_vec, params);
    spdlog::info("Number of tags: {}", testbench.get_number_of_injected_tags());

    auto instance = ducode::DesignInstance::create_instance(design);

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

    ///fill json object with relevant information about design -> then add it to result data json
    nlohmann::json ift_res = ducode::do_analysis(instance, design);
    const uint64_t number_of_simulated_tags = instance.m_signal_tag_map->get_vcd_data_size() * 32;

    if (testbench.get_number_of_injected_tags() > number_of_simulated_tags) {
      ift_res["number_of_injected_tags"] = number_of_simulated_tags;
    } else {
      ift_res["number_of_injected_tags"] = testbench.get_number_of_injected_tags();
    }

    auto end_overall = std::chrono::high_resolution_clock::now();
    result_data_random["overall_runtime"] = std::chrono::duration_cast<std::chrono::seconds>(
                                                end_overall - simulation_start)
                                                .count();
    result_data_random[designs.first] = ift_res;

    std::fstream os_json_random(
        "results/results_random_" + designs.first + "_" + std::to_string(static_cast<int>(coverage)) + ".json",
        std::ios::out);
    os_json_random << result_data_random;
    os_json_random.close();
  }
}
}// namespace

TEST_CASE("random_SPI_Master", "[exp_rand_res][.]") {
  std::pair<std::string, std::string> designs = {"SPI_Master", "clk"};
  run_experiment_random_resolution(designs);
}

TEST_CASE("random_SPI_Slave", "[exp_rand_res][.]") {
  std::pair<std::string, std::string> designs = {"SPI_Slave", "slaveSCLK"};
  run_experiment_random_resolution(designs);
}

TEST_CASE("random_y86", "[exp_rand_res][.]") {
  std::pair<std::string, std::string> designs = {"y86", "clk"};
  run_experiment_random_resolution(designs);
}

TEST_CASE("random_yadmc", "[exp_rand_res][.]") {
  std::pair<std::string, std::string> designs = {"yadmc", "sys_clk"};
  run_experiment_random_resolution(designs);
}

TEST_CASE("random_aes_128", "[exp_rand_res][.]") {
  std::pair<std::string, std::string> designs = {"aes_128", "clk"};
  run_experiment_random_resolution(designs);
}

TEST_CASE("random_usb2uart", "[exp_rand_res][.]") {
  std::pair<std::string, std::string> designs = {"usb2uart", "sys_clk"};
  run_experiment_random_resolution(designs);
}

TEST_CASE("random_openMSP430", "[exp_rand_res][.]") {
  std::pair<std::string, std::string> designs = {"openMSP430", "mclk"};
  run_experiment_random_resolution(designs);
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
