/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

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

#include <chrono>
#include <iostream>
#include <string>

namespace ducode {

/**
 * @brief Performs simulation of a design using the provided testbench and parameters.
 *
 * This function simulates a design using the given testbench module, VCD data, exported Verilog,
 * exported testbench, exported tags, tag map, and simulation parameters. It returns a DesignInstance
 * object containing the simulation results.
 *
 * If the tag map is empty, the function generates a testbench, simulates the design, and adds the
 * VCD data to the DesignInstance. If the tag map is not empty, the function performs multiple
 * simulations for each tag in the tag map. It exports the tags, generates a testbench with IFT,
 * simulates the design, and adds the VCD data to the DesignInstance for each simulation.
 *
 * The maximum duration for the simulations is set to 60 minutes. If the maximum duration is reached,
 * the function stops the simulations and returns the DesignInstance.
 *
 * @param design The design to be simulated.
 * @param tb_module The testbench module.
 * @param vcd_data The VCD data.
 * @param exported_verilog The path to export the Verilog file.
 * @param exported_testbench The path to export the testbench file.
 * @param exported_tags The path to export the tags file.
 * @param tag_map The map of tags.
 * @param params The simulation parameters.
 * @return A DesignInstance object containing the simulation results.
 */
inline DesignInstance do_simulation(Design& design, auto& tb_module, auto& vcd_data, auto& exported_verilog, auto& exported_testbench, auto& exported_tags, auto& tag_map, const nlohmann::json& params) {

  if (tag_map.empty()) {
    const ducode::Testbench testbench(design, vcd_data);
    testbench.write_verilog(exported_testbench, params);
    auto instance = DesignInstance::create_instance(design);
    auto data = ducode::simulate_design(exported_verilog, exported_testbench);
    instance.add_vcd_data(data);
    return instance;
  }

  auto encoded_tags = tag_processing(tb_module, tag_map);
  spdlog::info("Running {} simulations", encoded_tags.size());
  export_tags(tb_module, exported_tags.string(), encoded_tags[0]);

  //exporting testbench with IFT
  const ducode::Testbench testbench(design, vcd_data);
  testbench.write_verilog(exported_testbench, exported_tags, params);

  auto data = ducode::simulate_design(exported_verilog, exported_testbench, exported_tags);

  auto instance = DesignInstance::create_instance(design);
  instance.add_vcd_data(data);

  //exporting testbench with IFT again because from the second simulation on the testbench due to tag files having one more line
  if (encoded_tags.size() > 1) {
    export_tags(tb_module, exported_tags.string(), encoded_tags[1]);
    testbench.write_verilog(exported_testbench, exported_tags, params);
  } else {
    testbench.write_verilog(exported_testbench, params);
  }

  const uint32_t sim_max_duration = 60;
  auto max_time = std::chrono::system_clock::now() + std::chrono::minutes(sim_max_duration);

  /// here add loop which runs the simulation again for all created tag_map files.
  for (auto& tags: encoded_tags | std::ranges::views::drop(1)) {
    if (std::chrono::system_clock::now() >= max_time) {
      break;
    }
    export_tags(tb_module, exported_tags.string(), tags);
    testbench.write_verilog(exported_testbench, exported_tags, params);

    /// IFT simulation
    spdlog::info("Simulating Exported Design + Testbench");
    data = ducode::simulate_design(exported_verilog, exported_testbench, exported_tags);
    spdlog::info("End of Simulation");
    instance.add_vcd_data(data);
  }
  return instance;
}

/**
 * Performs simulation on the given design.
 *
 * @param design The design to be simulated.
 * @param vcd_data The VCD data for simulation.
 * @param exported_verilog The exported Verilog file path.
 * @param exported_testbench The exported testbench file path.
 * @param tagged_ports The tagged ports for simulation.
 * @param params The simulation parameters in JSON format.
 * @return The instance of the simulated design.
 */
inline DesignInstance do_simulation(Design& design, auto& vcd_data, auto& exported_verilog, auto& exported_testbench, auto& tagged_ports, const nlohmann::json& params) {

  const ducode::Testbench testbench(design, vcd_data);
  testbench.write_verilog(exported_testbench, tagged_ports, params);
  auto instance = DesignInstance::create_instance(design);
  auto data = ducode::simulate_design(exported_verilog, exported_testbench);
  instance.add_vcd_data(data);
  return instance;
}


}// namespace ducode