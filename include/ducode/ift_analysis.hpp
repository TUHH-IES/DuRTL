/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/design.hpp>
#include <ducode/instantiation_graph.hpp>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <chrono>
#include <cstdint>
#include <deque>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

/**
 * @brief Calculates the step size of a clock signal in a VCD file.
 * 
 * This function takes a VCD data object and the name of a clock signal as input.
 * It searches for the specified clock signal in the VCD data and retrieves its values.
 * The function then returns the step size of the clock signal, which is the time difference
 * between the third and second value in the signal's value history.
 * 
 * @param vcd_data A reference to the VCD data object.
 * @param clk_name The name of the clock signal.
 * @return The step size of the clock signal.
 */
inline double get_stepsize(auto& vcd_data, const std::string& clk_name) {

  std::deque<VCDTimedValue> clk_values;

  for (const VCDScope* scope: vcd_data->get_scopes().at(0).children) {
    for (const VCDSignal* signal: scope->signals) {
      if (signal->reference == clk_name) {
        clk_values = vcd_data->get_signal_values(signal->hash);
        break;
      }
    }
    if (!clk_values.empty()) {
      break;
    }
  }

  return clk_values[2].time;
}

/**
 * Counts the number of cells in a given design.
 *
 * @param design The design to count the cells in.
 * @return The total number of cells in the design.
 */
inline uint64_t count_number_of_cells_in_design(ducode::Design& design) {
  uint64_t count = 0;
  for (const auto& module: design.get_modules()) {
    count += module.get_cells().size();
  }
  return count;
}

/**
 * Counts the number of nets in a given design.
 *
 * @param design The design to count the nets in.
 * @return The total number of nets in the design.
 */
inline uint64_t count_number_of_nets_in_design(ducode::Design& design) {
  uint64_t count = 0;
  for (const auto& module: design.get_modules()) {
    count += module.get_nets().size();
  }
  return count;
}

inline nlohmann::json do_analysis(ducode::DesignInstance& instance, ducode::Design& design) {
  nlohmann::json ift_res;
  ift_res["nets"] = count_number_of_nets_in_design(design);
  ift_res["cells"] = count_number_of_cells_in_design(design);
  ift_res["inputs"] = design.get_module(design.get_top_module_name()).get_input_names().size();
  ift_res["outputs"] = design.get_module(design.get_top_module_name()).get_output_names().size();

  nlohmann::json path_json;

  auto start_analysis_ift = std::chrono::high_resolution_clock::now();

  std::deque<boost::adjacency_list<>::vertex_descriptor> path = instance.most_tagged_path();
  for (auto& p: path) {
    path_json.emplace_back(instance.m_graph.m_vertices[p].m_property.name);
    //      std::cout << instance.m_graph.m_vertices[p].m_property.name << "\n";
  }
  ift_res["path(most)"] = path_json;
  ift_res["path(most)_size"] = path.size();

  nlohmann::json path_least_json;
  std::deque<boost::adjacency_list<>::vertex_descriptor> path_least = instance.least_tagged_path();
  for (auto& p: path_least) {
    path_least_json.emplace_back(instance.m_graph.m_vertices[p].m_property.name);
    //      std::cout << instance.m_graph.m_vertices[p].m_property.name << "\n";
  }
  ift_res["path(least)"] = path_least_json;
  ift_res["path(least)_size"] = path_least.size();
  //  result_data_full[d] = ift_res;

  auto stop = std::chrono::high_resolution_clock::now();

  nlohmann::json path_most_toggled_json;
  const std::deque<boost::adjacency_list<>::vertex_descriptor> most_t_path = instance.most_toggled_path();
  for (const auto& p: most_t_path) {
    path_most_toggled_json.emplace_back(instance.m_graph.m_vertices[p].m_property.name);
    //      std::cout << instance.m_graph.m_vertices[p].m_property.name << "\n";
  }
  ift_res["path(most_toggled)"] = path_most_toggled_json;
  ift_res["path(most_toggled)_size"] = most_t_path.size();

  nlohmann::json path_least_toggled_json;
  const std::deque<boost::adjacency_list<>::vertex_descriptor> least_t_path = instance.least_toggled_path();
  for (const auto& p: least_t_path) {
    path_least_toggled_json.emplace_back(instance.m_graph.m_vertices[p].m_property.name);
    //      std::cout << instance.m_graph.m_vertices[p].m_property.name << "\n";
  }
  ift_res["path(least_toggled)"] = path_least_toggled_json;
  ift_res["path(least_toggled)_size"] = least_t_path.size();

  auto stop_toggle_metric = std::chrono::high_resolution_clock::now();

  ift_res["runtime_analysis_ift"] = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start_analysis_ift).count();
  ift_res["runtime_analysis_toggle"] = std::chrono::duration_cast<std::chrono::milliseconds>(stop_toggle_metric - stop).count();

  uint32_t total_tag_count = 0;
  std::vector<std::pair<std::string, uint32_t>> result_output_count = instance.count_output_tags();
  nlohmann::json output_tag_counts_json;
  for (auto& ele: result_output_count) {
    output_tag_counts_json[ele.first] = ele.second;
    total_tag_count += ele.second;
  }
  output_tag_counts_json["total_tag_count"] = total_tag_count;
  ift_res["output_tag_count"] = output_tag_counts_json;
  return ift_res;
}