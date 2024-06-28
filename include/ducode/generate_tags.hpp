/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/design.hpp>
#include <ducode/port.hpp>
#include <ducode/utility/concepts.hpp>
#include <ducode/utility/parser_utility.hpp>

#include <fmt/ranges.h>
#include <nlohmann/json.hpp>
#include <range/v3/view/enumerate.hpp>
#include <vcd-parser/VCDFile.hpp>

#include <random>
#include <string>

// NEW VERSION
/// export tag information from the tag_map for IFT simulation to tag_txt file
/// tag_map.key: tag_bit; tag_map.value: pair<timestep, port>
/// tag.txt lines have the form : input1_tag, input2_tag, ... , inputn_tag, #wait_time
/**
 * Exports tags to a file.
 *
 * This function exports tags to a specified file. It takes a module, a string representing the path to the file,
 * and a map of tags. The tags are written to the file in a specific format.
 *
 * @param module The module containing the tags.
 * @param exported_tags The path to the file where the tags will be exported.
 * @param tag_map A map of tags, where the key is the tag ID and the value is a pair of the tag's timestamp and port.
 *
 * @throws std::runtime_error if the file cannot be opened.
 */
inline void export_tags(const auto& module, const std::string& exported_tags, std::map<uint32_t, std::pair<uint32_t, ducode::Port>>& tag_map) {
  std::stringstream oss;

  Expects(!tag_map.empty());

  if (tag_map.begin()->second.first != 0) {// first line has no tags to set the time for the first given timestep
    for (auto& tag: module->get_ports()) {
      if (tag.m_direction == ducode::Port::Direction::input) {
        oss << std::bitset<tag_size>(0).to_string();
      }
    }
    oss << std::bitset<tag_size>((tag_map.begin()->second.first)).to_string() << "\n";
  }

  // collect all timesteps in current tag_map
  std::vector<uint32_t> timestamps;
  for (auto& timesteps: tag_map) {
    if (std::find(timestamps.begin(), timestamps.end(), timesteps.second.first) == timestamps.end()) {
      timestamps.emplace_back(timesteps.second.first);
    }
  }

  // collect tags and non-tags for each timestep
  // map<timestep, vector<tag_bit>>
  std::map<uint32_t, std::vector<uint32_t>> tags;
  for (auto& time: timestamps) {
    for (auto& ports: module->get_ports()) {
      if (ports.m_direction == ducode::Port::Direction::input) {
        bool tagged = false;
        for (auto& tag_port: tag_map) {
          if (tag_port.second.second.m_name == ports.m_name && tag_port.second.first == time) {
            tagged = true;
            tags[time].emplace_back(tag_port.first);
          }
        }
        if (!tagged) {
          tags[time].emplace_back(0);
        }
      }
    }
  }

  // print tags to file
  for (auto tgs = tags.begin(); tgs != tags.end(); tgs++) {
    for (auto& bits: tgs->second) {
      oss << std::bitset<tag_size>(bits).to_string();
    }
    if (std::next(tgs) != tags.end()) {
      oss << std::bitset<tag_size>((std::next(tgs)->first) - (tgs->first)).to_string() << "\n";
    } else {
      oss << std::bitset<tag_size>((tgs->first) - (std::prev(tgs)->first)).to_string() << "\n";
    }
  }

  // last line of the file sets the tags back to zero
  for (auto& tag: module->get_ports()) {
    if (tag.m_direction == ducode::Port::Direction::input) {
      oss << std::bitset<tag_size>(0).to_string();
    }
  }
  oss << std::bitset<tag_size>(0).to_string();

  spdlog::debug("end of input txt file");

  std::fstream os_tags(exported_tags, std::ios::out);
  if (!os_tags.is_open()) {
    throw std::runtime_error(fmt::format("Cannot open {}", exported_tags));
  }
  os_tags << oss.str();
  os_tags.close();
}

/**
 * Encodes the tags for each timestep and port in the given tag_map.
 * 
 * @param tag_map A map containing the tags for each timestep and port.
 * @return A map containing the encoded tags for each timestep and port.
 */
inline std::map<uint32_t, std::pair<uint32_t, ducode::Port>> encode_tags(std::map<uint32_t, std::vector<ducode::Port>>& tag_map) {
  std::bitset<tag_size> tag{1};
  std::map<uint32_t, std::pair<uint32_t, ducode::Port>> encoded_tags;
  for (auto& timestep: tag_map) {
    for (auto& port: timestep.second) {
      if (port.m_direction == ducode::Port::Direction::input) {
        encoded_tags[gsl::narrow<uint32_t>(tag.to_ulong())] = std::pair<uint32_t, ducode::Port>(timestep.first, port);
        tag <<= 1;
      }
    }
  }
  if (encoded_tags.size() > tag_size) {
    spdlog::error("encoded_tags map is larger than the size of the tag_vector");
  }
  return encoded_tags;
}

/**
 * Splits the given tag map into multiple simulation slices based on the number of timesteps per simulation run.
 *
 * @param module The module object.
 * @param tag_map The tag map to be split.
 * @return A vector of maps representing the simulation slices.
 */
inline std::vector<std::map<uint32_t, std::vector<ducode::Port>>> tag_map_splitting(const auto& module, std::map<uint32_t, std::vector<ducode::Port>>& tag_map) {

  std::map<uint32_t, std::vector<ducode::Port>> tmp;
  std::vector<std::map<uint32_t, std::vector<ducode::Port>>> simulation_slices;
  auto timesteps_per_simulation_run = tag_size / module->get_input_names().size();
  if (timesteps_per_simulation_run < 1) {
    timesteps_per_simulation_run = 1;
  }
  auto number_simulation_runs = tag_map.size() / timesteps_per_simulation_run;
  if (number_simulation_runs < 1) {
    number_simulation_runs = 1;
  }
  auto it = tag_map.begin();
  for (auto i = 0ul; i < number_simulation_runs; i++) {
    for (auto j = 0ul; j < timesteps_per_simulation_run; j++) {
      if (it != tag_map.end()) {
        tmp[it->first] = it->second;
        it++;
      }
    }
    simulation_slices.emplace_back(tmp);
    tmp.clear();
  }
  return simulation_slices;
}

/**
 * @brief Processes tags for a given module and tag map.
 *
 * This function takes a module and a tag map as input and processes the tags
 * to generate a vector of maps. Each map in the vector represents a set of
 * encoded tags for a specific tag map split.
 *
 * @param module The module for which tags need to be processed.
 * @param tag_map The tag map containing the tags to be processed.
 * @return A vector of maps, where each map represents a set of encoded tags.
 */
inline std::vector<std::map<uint32_t, std::pair<uint32_t, ducode::Port>>> tag_processing(const auto& module, std::map<uint32_t, std::vector<ducode::Port>>& tag_map) {
  std::vector<std::map<uint32_t, std::pair<uint32_t, ducode::Port>>> result;
  auto tag_map_split = tag_map_splitting(module, tag_map);
  result.reserve(tag_map_split.size());
  for (auto& t_map: tag_map_split) {
    result.emplace_back(encode_tags(t_map));
  }
  return result;
}

/**
 * Generates tags for full-resolution simulation.
 *
 * This function generates tags for full-resolution simulation based on the given parameters.
 * It creates a map where each entry represents a tag with the corresponding time it was inserted and for which input.
 *
 * @param final_time The final time of the simulation.
 * @param stepsize The step size of the simulation.
 * @param port_list The list of ports.
 * @return A map containing the generated tags.
 */
inline std::map<uint32_t, std::vector<ducode::Port>> generate_tags_full_resolution(double final_time, uint32_t stepsize, const std::vector<ducode::Port>& port_list) {
  // every entry represents a tag with the corresponding time it was inserted and for which input
  std::map<uint32_t, std::vector<ducode::Port>> tag_map;
  std::vector<ducode::Port> input_set;
  auto tag_cnt = 0ul;
  for (auto time = 0u; time <= gsl::narrow<std::size_t>(final_time); time += stepsize) {
    for (const auto& port: port_list)// Primary inputs
    {
      if (port.m_direction == ducode::Port::Direction::input) {
        input_set.emplace_back(port);
        tag_cnt++;
      }
    }
    tag_map[time] = input_set;
    input_set.clear();
  }
  spdlog::info("The number of tags injected by FULL-RESOLUTION: " + std::to_string(tag_cnt));
  return tag_map;
}


/**
 * Generates random tags for a given set of ports.
 *
 * This function generates a map of tags, where each tag represents a time and the corresponding ports
 * for which the tag was inserted. The tags are randomly generated based on the given parameters.
 *
 * @param final_time The final time for which tags are generated.
 * @param stepsize The step size between consecutive time points.
 * @param tag_cnt The number of tags to generate.
 * @param port_list The list of ports for which tags are generated.
 * @return A map of tags, where the key is the time and the value is a vector of ports.
 */
inline std::map<uint32_t, std::vector<ducode::Port>> generate_tags_random(double final_time, uint32_t stepsize, int tag_cnt, const std::vector<ducode::Port>& port_list) {
  auto random_bool = [distribution = std::uniform_int_distribution<>(0, 1)]() mutable -> bool {
    std::random_device device;
    std::mt19937 gen(device());
    auto result = distribution(gen);
    return result == 1;
  };
  auto nr_inputs = 0;
  for (const auto& net: port_list)// Primary inputs
  {
    if (net.m_direction == ducode::Port::Direction::input) {
      nr_inputs++;
    }
  }
  const auto max_nr_tags = (final_time / stepsize) * nr_inputs;
  if (tag_cnt > max_nr_tags) {
    spdlog::warn("Given Tag number larger than the possible number of tags!!");
    return generate_tags_full_resolution(final_time, stepsize, port_list);
  }
  // every entry represents a tag with the corresponding time it was inserted and for which input
  std::map<uint32_t, std::vector<ducode::Port>> tag_map;
  std::vector<ducode::Port> input_set;
  while (tag_cnt > 0) {
    for (uint32_t time = 0; time <= final_time; time += stepsize) {
      if (tag_map.find(time) != tag_map.end()) {
        input_set = tag_map[time];
      } else {
        input_set.clear();
      }
      for (const auto& net: port_list)// Primary inputs
      {
        if (net.m_direction == ducode::Port::Direction::input) {
          if (std::find(input_set.begin(), input_set.end(), net) == input_set.end()) {
            if (random_bool()) {
              input_set.emplace_back(net);
              tag_cnt--;
            }
          }
        }
        tag_map[time] = input_set;
        //        input_set.clear();
        if (tag_cnt == 0) {
          break;
        }
      }
      if (tag_cnt == 0) {
        break;
      }
    }
  }
  return tag_map;
}
