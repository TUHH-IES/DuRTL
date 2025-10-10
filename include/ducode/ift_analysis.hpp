/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/design.hpp>
#include <ducode/instantiation_graph.hpp>
#include <ducode/instantiation_graph_traits.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <vcd-parser/VCDTypes.hpp>

#include <chrono>
#include <cstdint>
#include <deque>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace ducode {

inline uint32_t value_vector_to_uint32(const VCDBitVector& value_vector) {
  uint32_t result = 0;
  const uint32_t max_size = 32;
  if (value_vector.size() > max_size) { throw std::runtime_error("VCDBitVector is too large for uint32_t"); }
  for (uint64_t i = 0; i < value_vector.size(); ++i) {
    if (value_vector[i] == VCDBit::VCD_X || value_vector[i] == VCDBit::VCD_Z) {
      throw std::runtime_error("VCDBitVector contains X or Z values!");
    }
    if (value_vector[i] == VCDBit::VCD_1) {
      result |= (1u << ((value_vector.size() - 1) - i));
    }
  }
  return result;
}

inline uint32_t value_vector_to_uint32(const SignalBitVector& value_vector) {
  uint32_t result = 0;
  const uint32_t max_size = 32;
  if (value_vector.size() > max_size) { throw std::runtime_error("VCDBitVector is too large for uint32_t"); }
  for (uint64_t i = 0; i < value_vector.size(); ++i) {
    if (value_vector[i] == SignalBit::BIT_X || value_vector[i] == SignalBit::BIT_Z) {
      throw std::runtime_error("VCDBitVector contains X or Z values!");
    }
    if (value_vector[i] == SignalBit::BIT_1) {
      result |= (1u << ((value_vector.size() - 1) - i));
    }
  }
  return result;
}

/**
 * Counts the number of cells in a given design.
 *
 * @param design The design to count the cells in.
 * @return The total number of cells in the design.
 */
inline uint64_t count_number_of_cells_in_design(Design& design) {
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
inline uint64_t count_number_of_nets_in_design(Design& design) {
  uint64_t count = 0;
  for (const auto& module: design.get_modules()) {
    count += module.get_nets().size();
  }
  return count;
}

inline nlohmann::json do_analysis(DesignInstance& instance, Design& design) {
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
  }
  ift_res["path(most)"] = path_json;
  ift_res["path(most)_size"] = path.size();

  nlohmann::json path_least_json;
  std::deque<boost::adjacency_list<>::vertex_descriptor> path_least = instance.least_tagged_path();
  for (auto& p: path_least) {
    path_least_json.emplace_back(instance.m_graph.m_vertices[p].m_property.name);
  }
  ift_res["path(least)"] = path_least_json;
  ift_res["path(least)_size"] = path_least.size();
  //  result_data_full[d] = ift_res;

  auto stop = std::chrono::high_resolution_clock::now();

  nlohmann::json path_most_toggled_json;
  const std::deque<boost::adjacency_list<>::vertex_descriptor> most_t_path = instance.most_toggled_path();
  for (const auto& p: most_t_path) {
    path_most_toggled_json.emplace_back(instance.m_graph.m_vertices[p].m_property.name);
  }
  ift_res["path(most_toggled)"] = path_most_toggled_json;
  ift_res["path(most_toggled)_size"] = most_t_path.size();

  nlohmann::json path_least_toggled_json;
  const std::deque<boost::adjacency_list<>::vertex_descriptor> least_t_path = instance.least_toggled_path();
  for (const auto& p: least_t_path) {
    path_least_toggled_json.emplace_back(instance.m_graph.m_vertices[p].m_property.name);
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

/**
 * @brief Get the number of IFT information flows in a given design instance.
 * 
 * This function takes a design instance as input and a map of information flows.
 * An information flow is defined as a connection between an output and an input signal in the design.
 * This function returns for each output the set of inputs that have an information flow to it at any point simulation.
 * @param instance The design instance to analyze.
 * @return A map of information flows in the design instance.
 */
[[nodiscard]] inline std::unordered_map<std::string, std::unordered_set<std::string>> get_ift_information_flows(DesignInstance& instance) {
  std::unordered_map<std::string, std::unordered_set<std::string>> ift_information_flows;

  auto input_names = instance.get_design().get_top_module().get_input_names();
  auto output_names = instance.get_design().get_top_module().get_output_names();
  std::vector<std::string> top_module_hierarchy = {instance.get_design().get_top_module_name()};
  std::vector<ducode::SignalIdentifier> input_ids;
  input_ids.reserve(input_names.size());
  for (const auto& input_name: input_names) {
    input_ids.emplace_back(ducode::SignalIdentifier{.name = input_name, .hierarchy = top_module_hierarchy});
  }
  ducode::SignalIdentifier target_id{.name = "output", .hierarchy = top_module_hierarchy};
  uint32_t timepoint = 0;
  ducode::TagSourceList ifs_test;
  for (const auto& output_name: output_names) {
    ifs_test = (instance.get_information_flows_for_output_from_inputs(input_ids, target_id));

    std::unordered_set<std::string> input_set;
    for (const auto& tag_source: ifs_test.m_tag_vector) {
      input_set.emplace(tag_source.input_id.name);
    }

    ift_information_flows[output_name] = input_set;
  }
  return ift_information_flows;
}

using IFT_info_set = std::unordered_set<IFInfo>;
using IFT_info_map = std::unordered_map<std::string, IFT_info_set>;

/**
 * @brief Get the number of IFT information flows in a given design instance.
 * 
 * This function takes a design instance as input and a map of information flows.
 * An information flow is defined as a connection between an output and an input signal in the design.
 * This function returns for each output the set of inputs that have an information flow to it at any point simulation.
 * @param instance The design instance to analyze.
 * @return A map of information flows in the design instance.
 */
[[nodiscard]] inline IFT_info_map get_ift_information_flows_complete(DesignInstance& instance) {
  IFT_info_map ift_information_flows;
  IFT_info_set input_info_set;

  auto input_names = instance.get_design().get_top_module().get_input_names();
  auto output_names = instance.get_design().get_top_module().get_output_names();
  std::vector<std::string> top_module_hierarchy = {instance.get_design().get_top_module_name()};
  std::vector<ducode::SignalIdentifier> input_ids;
  input_ids.reserve(input_names.size());
  for (const auto& input_name: input_names) {
    input_ids.emplace_back(ducode::SignalIdentifier{.name = input_name, .hierarchy = top_module_hierarchy});
  }
  ducode::SignalIdentifier target_id{.name = "output", .hierarchy = top_module_hierarchy};
  uint32_t timepoint = 0;
  ducode::TagSourceList ifs_test;
  for (const auto& output_name: output_names) {
    ifs_test = (instance.get_information_flows_for_output_from_inputs(input_ids, target_id));
    for (const auto& tag_source: ifs_test.m_tag_vector) {
      input_info_set.insert(tag_source);
    }
    ift_information_flows[output_name] = input_info_set;
  }
  return ift_information_flows;
}

[[nodiscard]] inline uint32_t get_IF_count(auto& ifs) {
  uint32_t if_count = 0;
  for (const auto& if_input_list: ifs) {
    for (const auto& if_input: if_input_list.second) {
      if_count++;
    }
  }
  return if_count;
}

}// namespace ducode
