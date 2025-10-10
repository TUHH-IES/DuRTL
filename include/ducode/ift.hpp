/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/design.hpp>
#include <ducode/ift_analysis.hpp>
#include <ducode/instantiation_graph.hpp>
#include <ducode/signals_data_manager.hpp>
#include <ducode/tag_generator.hpp>
#include <ducode/testbench.hpp>
#include <ducode/utility/iverilog_wrapper.hpp>
#include <ducode/utility/simulation.hpp>

#include <boost/filesystem/operations.hpp>
#include <nlohmann/json_fwd.hpp>
#include <spdlog/spdlog.h>

#include <vector>

// Standard Library
#include <algorithm>// std::ranges::find_if
#include <chrono>   // std::chrono::system_clock, std::chrono::minutes
#include <cstdint>  // uint32_t
#include <memory>   // std::shared_ptr, std::make_unique, std::make_shared
#include <sstream>  // std::stringstream

// Boost

// GSL
#include <gsl/assert>
#include <gsl/narrow>// gsl::narrow

// JSON (nlohmann)
#include <nlohmann/json.hpp>// nlohmann::json

// Project-specific
#include <ducode/utility/VCD_utility.hpp>
#include <ducode/utility/concepts.hpp>

#include <vcd-parser/VCDFile.hpp>
#include <vcd-parser/VCDFileParser.hpp>
#include <vcd-parser/VCDTypes.hpp>


namespace ducode {
/**
   * @brief Performs the IFT (Information Flow Tracking) simulation.
   *
   * This function takes a JSON file, a VCD file, and a step size as input parameters.
   * It performs the IFT simulation by parsing the JSON file, parsing the VCD file,
   * and generating the necessary verilog files and tag files for simulation.
   * It then simulates the exported design and testbench using the generated files.
   * The simulation is performed multiple times, each time with a different set of tags.
   * The results of each simulation are stored in a DesignInstance object.
   *
   * @param json_file The path to the JSON file containing the design description.
   * @param vcd_file The path to the VCD file containing the simulation data.
   * @param stepsize The step size for generating tags.
   */
template<filesystem_like T>
void ift(const T &json_file, const T &vcd_file, nlohmann::json &params) {
  Expects(params["ift"] == true);
  Expects(params.contains("tag_size"));
  Expects(params.contains("step_size"));

  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  Expects(created);

  spdlog::debug("Temp dir: {}", temp_dir.string());
  ducode::Design design = ducode::Design::parse_json(json_file);

  VCDFileParser vcd_obj;
  auto vcd_data = vcd_obj.parse_file(vcd_file.string());
  auto exported_verilog = temp_dir / "ift_design.v";
  auto exported_testbench = temp_dir / "ift_testbench.v";
  auto exported_table = temp_dir / "table.tex";

  design.write_verilog(exported_verilog, params);

  auto top_module = std::ranges::find_if(design.get_modules(), [&](const auto &mod) {
    return mod.get_name() == design.get_top_module_name();
  });

  const auto max_tag_number = 100;
  const uint32_t tag_size = params.value("tag_size", 512u);
  const auto stepsize = params.value("stepsize", 1);
  const auto timesteps_per_simulation_run = ducode::get_timesteps_per_simulation_run(*top_module, tag_size);
  const auto number_simulation_runs = ducode::get_number_of_simulation_runs(stepsize, timesteps_per_simulation_run,
                                                                            vcd_data);
  const uint32_t coverage = 20;
  //exporting testbench with IFT
  const VCDScope *root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));
  testbench.set_tag_generator(
      std::make_unique<ducode::DeterministicRandomTagGenerator>(top_module->get_input_ports(), vcd_data, coverage));
  testbench.write_verilog(exported_testbench, params);

  auto exported_tags_files = ducode::get_exported_tags_files(number_simulation_runs);
  auto tags_times = ducode::get_tags_times(vcd_data, gsl::narrow<uint32_t>(stepsize));
  auto exported_tags_vec = ducode::get_exported_tags_vec(exported_tags_files, number_simulation_runs,
                                                         tags_times, timesteps_per_simulation_run);
  testbench.write_tags(exported_tags_vec, params);

  auto instance = ducode::DesignInstance::create_instance(design);

  constexpr uint32_t sim_max_duration = 60;
  const auto max_time = std::chrono::system_clock::now() + std::chrono::minutes(sim_max_duration);

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vec;
  for (const auto &exported_tags: exported_tags_files) {
    /// IFT simulation
    spdlog::info("Simulating Exported Design + Testbench");
    auto data = ducode::simulate_design(exported_verilog, exported_testbench, exported_tags);
    spdlog::info("End of Simulation");
    vcd_data_vec.emplace_back(data);
  }

  std::vector<const VCDScope *> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vec.size());
  for (auto &vcd_file_: vcd_data_vec) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file_, design));
  }

  auto test_value_map = std::make_shared<ducode::VCDSignalsDataManager>(vcd_data_vec, vcd_root_scopes, vcd_data_vec.size());
  instance.add_signal_tag_map(test_value_map);

  auto results = do_analysis(instance, design);
  std::stringstream results_string;
  results_string << results;
  spdlog::info("{}", results_string.str());
}
}// namespace ducode
