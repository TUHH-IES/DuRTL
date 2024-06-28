/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/generate_tags.hpp>
#include <ducode/ift_analysis.hpp>
#include <ducode/instantiation_graph.hpp>
#include <ducode/sim.hpp>
#include <ducode/utility/iverilog_wrapper.hpp>

#include <boost/filesystem/operations.hpp>
#include <catch2/catch_all.hpp>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <vcd-parser/VCDComparisons.hpp>

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

/*
 * Contains all experiments for the IFT_Analysis paper
 */

void run_experiment_full_resolution(const std::pair<std::string, std::string>& designs) {
  nlohmann::json params;
  auto start_overall = std::chrono::high_resolution_clock::now();

  nlohmann::json result_data_full;
  params["ift"] = true;

  auto json_file = std::filesystem::path(TESTFILES_DIR) / designs.first / (designs.first + ".json");
  auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / designs.first / (designs.first + ".vcd");

  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  Expects(created);

  auto exported_verilog = temp_dir / ("ift" + designs.first + ".v");
  auto exported_testbench = temp_dir / ("ift" + designs.first + "_tb.v");
  auto exported_tags = temp_dir / "vcd_input.txt";

  ducode::Design design = ducode::Design::parse_json(json_file);

  VCDFileParser vcd_obj;
  spdlog::info("Loading Original Simulation Data");
  auto vcd_data = vcd_obj.parse_file(vcd_reference_file_path.string());

  //export the design with IFT
  design.write_verilog(exported_verilog, params);

  //get top module
  auto tb_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });

  //create tags with full resolution
  auto tag_map = generate_tags_full_resolution(vcd_data->get_timestamps().back(), static_cast<uint32_t>(get_stepsize(vcd_data, designs.second)), tb_module->get_ports());

  ducode::DesignInstance instance = ducode::do_simulation(design, tb_module, vcd_data, exported_verilog, exported_testbench, exported_tags, tag_map, params);

  //    auto stop_ift_simulation = std::chrono::high_resolution_clock::now();

  ///fill json object with relevant information about design -> then add it to result data json
  nlohmann::json ift_res = do_analysis(instance, design);
  const uint32_t number_of_injected_tags_total = static_cast<uint32_t>((vcd_data->get_timestamps().back() / get_stepsize(vcd_data, designs.second)) * static_cast<double>(tb_module->get_input_names().size()));
  const uint64_t number_of_simulated_tags = instance.m_vcd_data.size() * 32;

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

void run_experiment_random_resolution(const std::pair<std::string, std::string>& designs) {
  nlohmann::json params;
  //  auto start_overall = std::chrono::high_resolution_clock::now();

  std::map<uint32_t, std::vector<std::pair<std::string, uint32_t>>> const tag_count_map;
  nlohmann::json result_data_random;

  params["ift"] = false;

  auto json_file = std::filesystem::path(TESTFILES_DIR) / designs.first / (designs.first + ".json");
  auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / designs.first / (designs.first + ".vcd");

  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  auto exported_verilog = temp_dir / ("ift" + designs.first + ".v");
  auto exported_testbench = temp_dir / ("ift" + designs.first + "_tb.v");
  auto exported_tags = temp_dir / "vcd_input.txt";

  ducode::Design design = ducode::Design::parse_json(json_file);
  //export the design with IFT
  design.write_verilog(exported_verilog, params);

  VCDFileParser vcd_obj;
  spdlog::info("Loading Original Simulation Data");
  auto vcd_data = vcd_obj.parse_file(vcd_reference_file_path.string());

  //get top module
  auto tb_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });

  const uint64_t max_tag_number = static_cast<uint64_t>((vcd_data->get_timestamps().back() / get_stepsize(vcd_data, designs.second)) * static_cast<double>(tb_module->get_input_names().size()));
  const uint64_t tag_number_increase = max_tag_number / 4;

  const int add_nr = 25;
  float tnr = add_nr;

  for (uint64_t tag_number = max_tag_number / 4; tag_number <= max_tag_number; tag_number += tag_number_increase) {
    auto simulation_start = std::chrono::high_resolution_clock::now();
    std::map<std::string, std::vector<uint32_t>> const tag_counts;

    params["ift"] = true;

    design.write_verilog(exported_verilog, params);

    auto tag_map = generate_tags_random(vcd_data->get_timestamps().back(), static_cast<uint32_t>(get_stepsize(vcd_data, designs.second)), static_cast<int>(tag_number), tb_module->get_ports());
    uint32_t tag_cnt = 0;
    for (auto& tags: tag_map) {
      for (auto& tss: tags.second) {
        tag_cnt++;
      }
    }
    spdlog::info("Number of tags: {}", tag_cnt);

    ducode::DesignInstance instance = ducode::do_simulation(design, tb_module, vcd_data, exported_verilog, exported_testbench, exported_tags, tag_map, params);

    ///fill json object with relevant information about design -> then add it to result data json
    nlohmann::json ift_res = do_analysis(instance, design);
    const uint64_t number_of_simulated_tags = instance.m_vcd_data.size() * 32;

    if (tag_number > number_of_simulated_tags) {
      ift_res["number_of_injected_tags"] = number_of_simulated_tags;
    } else {
      ift_res["number_of_injected_tags"] = tag_number;
    }

    auto end_overall = std::chrono::high_resolution_clock::now();
    result_data_random["overall_runtime"] = std::chrono::duration_cast<std::chrono::seconds>(end_overall - simulation_start).count();
    result_data_random[designs.first] = ift_res;

    std::fstream os_json_random("results/results_random_" + designs.first + "_" + std::to_string(static_cast<int>(tnr)) + ".json", std::ios::out);
    os_json_random << result_data_random;
    os_json_random.close();
    tnr += add_nr;
  }
}

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

// TEST_CASE("random_mips_16_core_top", "[exp_rand_res][.]") {
//   std::pair<std::string, std::string> designs = {"mips_16_core_top", "clk"};
//   run_experiment_random_resolution(designs);
// }

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