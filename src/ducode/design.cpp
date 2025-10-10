/* SPDX-License-Identifier: Apache-2.0 */

#include <ducode/design.hpp>
#include <ducode/module.hpp>
#include <ducode/net.hpp>
#include <ducode/port.hpp>
#include <ducode/utility/json.hpp>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/graph/detail/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/properties.hpp>
#include <fmt/core.h>
#include <gsl/assert>
#include <gsl/narrow>
#include <nlohmann/json_fwd.hpp>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

namespace ducode {

namespace {
void add_attributes(Module& module, const nlohmann::json& module_json) {
  module.get_attributes() = module_json["attributes"];

  for (const auto& attribute: module_json["attributes"].items()) {
    if (attribute.key() == "cells_not_processed") {
      spdlog::warn("The module has unprocessed cells");
    }
  }
}

void add_nets(Module& module, const nlohmann::json& module_json) {
  Ensures(module_json.contains("netnames"));
  for (const auto& [netname_key, netname]: module_json["netnames"].items()) {
    const bool hidden = (netname["hide_name"].get<int>() == 1);
    std::string src_name;
    std::string init;
    uint32_t line = 0;
    for (const auto& attribute: netname["attributes"].items()) {
      if (attribute.key() == "src") {
        const auto& src = attribute.value().get<std::string>();
        std::vector<std::string> result;
        boost::split(result, src, boost::is_any_of(":"));
        line = gsl::narrow<uint32_t>(std::stoul(result.back()));
        result.erase(--result.end());
        src_name = boost::algorithm::join(result, ":");
      } else if (attribute.key() == "init") {
        init = attribute.value().get<std::string>();
      } else {
        spdlog::trace("Netname attribute not managed: {}", attribute.key());
      }
    }

    auto bits = to_bit_vector(netname["bits"]);

    Net net(netname_key, hidden, bits);
    net.set_source_info(src_name, line);
    net.set_init_value(init);

    //We have nets that are unique in the bits they represent
    //The same net may have multiple aliases
    module.add_net(net);
  }
}

void add_ports(Module& module, const nlohmann::json& module_json) {
  if (!module_json.contains("ports")) {
    throw std::runtime_error("No ports in this module");
  }

  for (const auto& [port_name, port]: module_json["ports"].items()) {
    auto bits = to_bit_vector(port["bits"]);
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
    module.add_port(port_name, port_direction, bits);
  }
}

void add_cells(Module& module, const nlohmann::json& module_json, const std::unordered_set<std::string>& modules) {
  Ensures(module_json.contains("cells"));
  for (const auto& [cell_key, cell_json]: module_json["cells"].items()) {
    const std::string type = cell_json["type"].get<std::string>();
    const bool hidden = (cell_json["hide_name"].get<int>() == 1);

    std::vector<Port> ports;
    for (const auto& [port_name, port_direction_json]: cell_json["port_directions"].items()) {
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
      const auto port_bits = cell_json["connections"].find(port_name).value();

      // Parameters width
      if (cell_json["parameters"].contains(port_name + "_WIDTH")) {
        const auto& port_width = cell_json["parameters"][port_name + "_WIDTH"];
        size_t width = 0;
        if (port_width.is_number_integer()) {
          width = port_width.get<size_t>();
        } else {
          width = std::stoul(port_width.get<std::string>(), nullptr, 2);
        }
        Ensures(width == port_bits.size());
      }

      // Parameters signedness
      bool port_signed = false;
      if (port_direction == Port::Direction::input && cell_json["parameters"].contains(port_name + "_SIGNED")) {
        const auto& jsign = cell_json["parameters"][port_name + "_SIGNED"];
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
      ports.emplace_back(port_name, "", port_direction, to_bit_vector(port_bits), port_signed);
    }

    module.add_cell(modules, cell_key, type, ports, hidden, cell_json.at("parameters"), cell_json.at("attributes"));
    spdlog::debug("Cell {} added to module {}", cell_key, module.get_name());
  }
}
}// namespace

namespace detail {
Design parse_json(const std::string& json) {
  std::stringstream json_stream(json);
  nlohmann::json json_content = nlohmann::json::parse(json_stream);

  if (!json_content.contains("modules")) {
    throw std::runtime_error("json does not contain modules");
  }

  Design design;

  std::unordered_set<std::string> modules;

  // Extract all modules in the file
  for (const auto& [module_key, module_json]: json_content["modules"].items()) {
    modules.emplace(module_key);
  }

  for (const auto& [module_key, module_json]: json_content["modules"].items()) {
    design.m_modules.emplace_back(module_key);
    auto& module = design.m_modules.back();

    if (module_json.contains("attributes")) {
      add_attributes(module, module_json);
    }

    add_nets(module, module_json);
    add_ports(module, module_json);
    add_cells(module, module_json, modules);
    module.populate_graph();

    spdlog::debug("Module {} added to the design", module.get_name());
  }

  nlohmann::json root_module;
  std::string root_module_name;

  auto root_vertex = boost::add_vertex(Design::VertexProperty{"root"}, design.m_hierarchy);
  if (json_content["modules"].size() == 1) {
    root_module = json_content["modules"].begin().value();
    root_module_name = json_content["modules"].begin().key();
  }

  if (root_module_name.empty()) {
    auto root_iterator = std::ranges::find_if(json_content["modules"].items(), [](const auto& module) {
      const auto& [module_key, module_json] = module;
      return module_json["attributes"].contains("top");
    });

    if (root_iterator == json_content["modules"].items().end()) {
      throw std::runtime_error("Json does not specify a root module");
    }

    root_module = root_iterator.value();
    root_module_name = root_iterator.key();
  }

  std::function<void(const nlohmann::json&, Design::HierarchyGraph::vertex_descriptor)> populate_hierarchy;
  populate_hierarchy = [&modules, &json_content, &graph = design.m_hierarchy, &populate_hierarchy](const nlohmann::json& current_module, Design::HierarchyGraph::vertex_descriptor current_vertex) {
    for (const auto& [cell_key, cell_json]: current_module["cells"].items()) {
      const std::string cell_type = cell_json["type"].get<std::string>();
      if (modules.contains(cell_type)) {
        const auto& module_json = json_content["modules"][cell_type];
        auto child_vertex = boost::add_vertex(Design::VertexProperty{cell_key}, graph);
        boost::add_edge(current_vertex, child_vertex, graph);
        populate_hierarchy(module_json, child_vertex);
      }
    }
  };

  design.set_top_module_name(root_module_name);
  populate_hierarchy(root_module, root_vertex);
  auto file = std::ofstream(fmt::format("{}.dot", root_module_name), std::ofstream::out);
  write_graphviz(file, design.m_hierarchy, make_label_writer(boost::get(boost::vertex_name_t(), design.m_hierarchy)));

  return design;
}
}// namespace detail
}// namespace ducode