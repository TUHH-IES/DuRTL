/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/generate_tags.hpp>
#include <ducode/ift_analysis.hpp>
#include <ducode/instantiation_graph.hpp>
#include <ducode/testbench.hpp>
#include <ducode/utility/iverilog_wrapper.hpp>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <iostream>
#include <string>
#include <vector>

namespace ducode {
template<filesystem_like T>
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
inline void ift(const T& json_file, const T& vcd_file, uint32_t stepsize) {
  nlohmann::json params;
  params["ift"] = true;

  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  Expects(created);

  spdlog::debug("Temp dir: {}", temp_dir.string());
  ducode::Design design = ducode::Design::parse_json(json_file);

  VCDFileParser vcd_obj;
  auto vcd_data = vcd_obj.parse_file(vcd_file.string());
  auto exported_verilog = temp_dir / "ift_design.v";
  auto exported_testbench = temp_dir / "ift_testbench.v";
  auto exported_tags = temp_dir / "vcd_input.txt";
  auto exported_table = temp_dir / "table.tex";

  design.write_verilog(exported_verilog, params);

  auto tb_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });

  //  auto tag_map_new = generate_tags_full_resolution(tb_module, vcd_data->get_timestamps().back(), stepsize);
  const auto max_tag_number = 100;
  auto tag_map_new = generate_tags_random(vcd_data->get_timestamps().back(), stepsize, max_tag_number, tb_module->get_ports());
  std::vector<std::map<uint32_t, std::pair<uint32_t, ducode::Port>>> encoded_tags = tag_processing(tb_module, tag_map_new);
  spdlog::info("Executing {} simulations", encoded_tags.size());
  export_tags(tb_module, exported_tags.string(), encoded_tags[0]);

  const ducode::Testbench testbench(design, vcd_data);
  testbench.write_verilog(exported_testbench, exported_tags, params);

  spdlog::info("Simulating Exported Design + Testbench");
  auto data = ducode::simulate_design(exported_verilog, exported_testbench, exported_tags);
  spdlog::info("End of Simulation");

  auto instance = ducode::DesignInstance::create_instance(design);
  instance.add_vcd_data(data);

  // exporting testbench with IFT again because from the second simulation on the testbench due to tag files having one more line
  export_tags(tb_module, exported_tags.string(), encoded_tags[1]);
  testbench.write_verilog(exported_testbench, exported_tags, params);

  // here add loop which runs the simulation again for all created tag_map files.
  for (auto& tags: encoded_tags) {
    if (tags == encoded_tags[0]) {
      continue;
    }
    export_tags(tb_module, exported_tags.string(), tags);

    // IFT simulation
    spdlog::info("Simulating Exported Design + Testbench");
    data = ducode::simulate_design(exported_verilog, exported_testbench, exported_tags);
    spdlog::info("End of Simulation");
    instance.add_vcd_data(data);
  }

  auto results = do_analysis(instance, design);
  std::stringstream results_string;
  results_string << results;
  spdlog::info("{}", results_string.str());
}
}// namespace ducode