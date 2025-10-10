/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/module.hpp>
#include <ducode/utility/concepts.hpp>

#include "ducode/instantiation_graph_traits.hpp"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_selectors.hpp>
#include <boost/graph/properties.hpp>
#include <boost/pending/property.hpp>
#include <fmt/core.h>
#include <nlohmann/json_fwd.hpp>

#include <algorithm>
#include <fstream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace ducode {

class Design;

namespace detail {
Design parse_json(const std::string& json);
}// namespace detail

/**
 * @brief Represents a design that consists of modules, nets, and cells.
 * 

 * It also supports parsing a design from a JSON file.
 */
class Design {
  std::string m_top_module_name;
  std::string m_clk_signal_name;
  std::vector<Module> m_modules;

  using VertexProperty = boost::property<boost::vertex_name_t, std::string>;
  using HierarchyGraph = boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, VertexProperty>;
  HierarchyGraph m_hierarchy;

public:
  [[nodiscard]] const std::vector<Module>& get_modules() const {
    return m_modules;
  }

  [[nodiscard]] std::string get_top_module_name() const { return m_top_module_name; };

  [[nodiscard]] std::string get_clk_signal_name() const { return m_clk_signal_name; };

  /**
   * @brief Retrieves a module by its name.
   * 
   * This function searches for a module with the specified name in the collection of modules.
   * If a module with the given name is found, it is returned.
   * If no module with the given name is found, a std::runtime_error is thrown.
   * 
   * @param name The name of the module to retrieve.
   * @return const Module& A reference to the found module.
   * @throws std::runtime_error if the module with the given name is not found.
   */
  [[nodiscard]] const Module& get_module(const std::string& name) const {
    auto item = std::ranges::find_if(m_modules, [&](const auto& module) { return module.get_name() == name; });
    if (item == m_modules.end()) {
      throw std::runtime_error(fmt::format("Module {} not found!", name));
    }
    return *item;
  }

  [[nodiscard]] const Module& get_top_module() const {
    if (m_top_module_name.empty()) {
      throw std::runtime_error("Top module name is not set");
    }
    return get_module(m_top_module_name);
  }

  [[nodiscard]] std::vector<SignalIdentifier> get_input_signal_ids() const {
    std::vector<SignalIdentifier> input_signal_ids;
    const auto& t_module = get_top_module();
    for (const auto& input_name: t_module.get_input_names()) {
      input_signal_ids.push_back({input_name, {t_module.get_name()}});
    }
    return input_signal_ids;
  }


  [[nodiscard]] std::vector<SignalIdentifier> get_output_signal_ids() const {
    std::vector<SignalIdentifier> input_signal_ids;
    const auto& t_module = get_top_module();
    for (const auto& input_name: t_module.get_output_names()) {
      input_signal_ids.push_back({input_name, {t_module.get_name()}});
    }
    return input_signal_ids;
  }


  void set_top_module_name(std::string name) {
    m_top_module_name = std::move(name);
  }

  void set_clk_signal_name(std::string name) {
    m_clk_signal_name = std::move(name);
  }

  [[nodiscard]] bool check_clock_signal_consistency() const {
    auto top_module = get_top_module();
    return top_module.check_clock_signal_consistency();
  }
  /**
   * Writes the Verilog code to the specified file.
   *
   * @param filename The name of the file to write the Verilog code to.
   * @param params   The JSON object containing the parameters for generating the Verilog code.
   */
  template<filesystem_like T>
  void write_verilog(const T& filename, const nlohmann::json& params) {
    return write_verilog(filename.string(), params);
  }

  /**
   * Writes the design to a Verilog file.
   *
   * @param filename The name of the Verilog file to write.
   * @param params The parameters for exporting the design.
   * @throws std::runtime_error if the file cannot be opened.
   */
  void write_verilog(const std::string& filename, const nlohmann::json& params) const {
    auto file = std::ofstream(filename, std::ofstream::out);
    if (!file.is_open()) {
      throw std::runtime_error(fmt::format("Cannot open {}", filename));
    }
    file << export_design(params);
  }

  /**
   * Exports the design to a string in Verilog format.
   *
   * @param params The parameters for exporting the design.
   * @return A string containing the exported Verilog code.
   */
  [[nodiscard]] std::string export_design(const nlohmann::json& params) const {
    std::stringstream os;
    for (const auto& mod: m_modules) {
      os << mod.export_verilog(params) << '\n';
    }
    return os.str();
  }

  /**
   * Parses a JSON file and returns a Design object.
   *
   * @param filename The name of the JSON file to parse.
   * @return The parsed Design object.
   */
  template<filesystem_like T>
  static Design parse_json(const T& filename) {
    spdlog::debug("Json file: {}", filename.string());
    std::ifstream json_file(filename.string());

    if (!json_file.is_open()) {
      throw std::runtime_error(fmt::format("File '{}' could not be opened", filename.string()));
    }

    std::stringstream json;
    json << json_file.rdbuf();

    return detail::parse_json(json.str());
  }

  [[nodiscard]] const HierarchyGraph& get_hierarchy_graph() const { return m_hierarchy; }

  friend Design detail::parse_json(const std::string& json);
};

}// namespace ducode
