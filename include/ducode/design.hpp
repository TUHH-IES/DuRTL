/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/module.hpp>
#include <ducode/utility/concepts.hpp>
#include <ducode/utility/json.hpp>

#include <boost/algorithm/string.hpp>
#include <vcd-parser/VCDFile.hpp>

#include <chrono>
#include <ostream>
#include <random>
#include <string>
#include <utility>
#include <vector>

namespace ducode {
/**
 * @brief Represents a design that consists of modules, nets, and cells.
 * 
 * The `Design` class provides methods to access and manipulate the modules, as well as write the design to a Verilog file.
 * It also supports parsing a design from a JSON file.
 */
class Design {
  std::string m_filename;
  std::string m_top_module_name;
  std::vector<Module> m_modules;

public:
  /**
   * @brief Retrieves the list of modules.
   * 
   * This function returns a constant reference to the vector of modules.
   * 
   * @return A constant reference to the vector of modules.
   */
  [[nodiscard]] const std::vector<Module>& get_modules() const {
    return m_modules;
  }

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

  /**
   * @brief Returns the filename associated with the object.
   *
   * This function returns a constant reference to the filename associated with the object.
   *
   * @return A constant reference to the filename.
   */
  [[nodiscard]] const std::string& get_filename() const { return m_filename; };

  /**
   * @brief Sets the name of the top module.
   *
   * This function sets the name of the top module in the design.
   *
   * @param name The name of the top module.
   */
  void set_top_module_name(std::string name) {
    m_top_module_name = std::move(name);
  }

  /**
   * @brief Get the name of the top module.
   * 
   * @return The name of the top module as a string.
   */
  [[nodiscard]] std::string get_top_module_name() const { return m_top_module_name; };

  template<filesystem_like T>
  /**
   * Writes the Verilog code to the specified file.
   *
   * @param filename The name of the file to write the Verilog code to.
   * @param params   The JSON object containing the parameters for generating the Verilog code.
   */
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

  template<filesystem_like T>
  /**
   * Parses a JSON file and returns a Design object.
   *
   * @param filename The name of the JSON file to parse.
   * @return The parsed Design object.
   */
  static Design parse_json(const T& filename) {
    return parse_json(filename.string());
  }

  /**
   * Parses a JSON file and constructs a Design object from its contents.
   *
   * @param filename The path to the JSON file to be parsed.
   * @return The Design object constructed from the JSON file.
   * @throws std::runtime_error if the file cannot be opened or if the JSON does not contain the required elements.
   */
  static Design parse_json(const std::string& filename) {
    spdlog::debug("Json file: {}", filename);
    std::ifstream json_file(filename);

    if (!json_file.is_open()) {
      throw std::runtime_error(fmt::format("File '{}' could not be opened", filename));
    }

    nlohmann::json json_content = nlohmann::json::parse(json_file);

    if (!json_content.contains("modules")) {
      throw std::runtime_error("json does not contain modules");
    }

    Design duDesign;
    duDesign.m_filename = filename;

    std::unordered_set<std::string> modules;

    // Extract all modules in the file
    for (const auto& [module_key, module]: json_content["modules"].items()) {
      modules.emplace(module_key);
    }

    for (const auto& [module_key, module]: json_content["modules"].items()) {
      std::string module_src_name;
      uint32_t line = 0;
      duDesign.m_modules.emplace_back(module_key);
      auto& duMod = duDesign.m_modules.back();

      if (module.contains("attributes")) {
        for (const auto& attribute: module["attributes"].items()) {
          if (attribute.key() == "cells_not_processed") {
            spdlog::warn("The module {} at {} has unprocessed cells", module_key, filename);
          } else if (attribute.key() == "src") {
            const auto& src = attribute.value().get<std::string>();
            auto colon_pos = src.find(':');
            module_src_name = src.substr(0, colon_pos);
            line = gsl::narrow<uint32_t>(std::stoul(src.substr(colon_pos + 1, src.size())));
            duMod.set_source_info(module_src_name, line);
          } else if (attribute.key() == "top") {
            duDesign.set_top_module_name(module_key);
          } else {
            spdlog::trace("Module attribute not managed: {}", attribute.key());
          }
        }
      }

      // Extract all nets at first
      Ensures(module.contains("netnames"));
      for (const auto& [netname_key, netname]: module["netnames"].items()) {
        const bool hidden = (netname["hide_name"].get<int>() == 1);
        std::string src_name;
        std::string init;
        line = 0;
        for (const auto& attribute: netname["attributes"].items()) {
          if (attribute.key() == "src") {
            const auto& src = attribute.value().get<std::string>();
            auto colon_pos = src.find(':');
            src_name = src.substr(0, colon_pos);
            line = gsl::narrow<uint32_t>(std::stoul(src.substr(colon_pos + 1, src.size())));
          } else if (attribute.key() == "init") {
            init = attribute.value().get<std::string>();
          } else {
            spdlog::trace("Netname attribute not managed: {}", attribute.key());
          }
        }

        auto duBits = to_bit_vector(netname["bits"]);

        Net duNet(netname_key, hidden, duBits);
        duNet.set_source_info(src_name, line);
        duNet.set_init_value(init);

        //We have nets that are unique in the bits they represent
        //The same net may have multiple aliases
        duMod.add_net(duNet);
      }

      // Get all the ports for this module
      Ensures(module.contains("ports"));
      for (const auto& [port_name, port]: module["ports"].items()) {
        auto duBits = to_bit_vector(port["bits"]);
        const Net duNet(port_name, false, duBits);

        const auto& type = port["direction"].get<std::string>();

        Port::Direction port_direction = Port::Direction::unknown;
        if (type == "input") {
          port_direction = Port::Direction::input;
        } else if (type == "output") {
          port_direction = Port::Direction::output;
        } else if (type == "inout") {
          port_direction = Port::Direction::inout;
        } else {
          throw std::runtime_error("Port type is not supported");
        }
        duMod.add_port(port_name, port_direction, duBits);
        /** \todo Ports can be signed, see e.g., \c negator.v */
      }

      // Get all the cells of this module
      Ensures(module.contains("cells"));
      for (const auto& [cell_key, cell]: module["cells"].items()) {
        const std::string type = cell["type"].get<std::string>();
        const bool hidden = (cell["hide_name"].get<int>() == 1);

        std::vector<Port> ports;
        for (const auto& [port_name, port_direction_json]: cell["port_directions"].items()) {
          Port::Direction port_direction = Port::Direction::unknown;

          if (port_direction_json.get<std::string>() == "input") {
            port_direction = Port::Direction::input;
          } else if (port_direction_json.get<std::string>() == "output") {
            port_direction = Port::Direction::output;
          } else if (port_direction_json.get<std::string>() == "inout") {
            port_direction = Port::Direction::inout;
          } else {
            throw std::runtime_error("Port type is not supported");
          }

          // Port bits
          const auto jbits = cell["connections"].find(port_name).value();

          // Parameters width
          if (cell["parameters"].contains(port_name + "_WIDTH")) {
            const auto& jwidth = cell["parameters"][port_name + "_WIDTH"];
            size_t width = 0;
            if (jwidth.is_number_integer()) {
              width = jwidth.get<size_t>();
            } else {
              width = std::stoul(jwidth.get<std::string>(), nullptr, 2);
            }
            Ensures(width == jbits.size());
          }

          // Parameters signedness
          bool port_signed = false;
          if (port_direction == Port::Direction::input && cell["parameters"].contains(port_name + "_SIGNED")) {
            const auto& jsign = cell["parameters"][port_name + "_SIGNED"];
            int sign = 0;
            if (jsign.is_number_integer()) {
              sign = jsign.get<int>();
            } else {
              sign = std::stoi(jsign.get<std::string>(), nullptr, 2);
            }
            Ensures(sign == 0 || sign == 1);
            port_signed = sign != 0;
          }

          // Add port
          ports.emplace_back(port_name, "", port_direction, to_bit_vector(jbits), port_signed);
        }

        duMod.add_cell(modules, cell_key, type, ports, hidden, cell.at("parameters"), cell.at("attributes"));
        spdlog::debug("Cell {} added to module {}", cell_key, duMod.get_name());
      }
      duMod.populate_graph();
      spdlog::debug("---- Module {} added to the design --------------\n", duMod.get_name());
    }

    return duDesign;
  }
};

}// namespace ducode