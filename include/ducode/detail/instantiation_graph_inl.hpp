/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/utility/contains.hpp>

#include <range/v3/algorithm/count_if.hpp>
#include <vcd-parser/VCDComparisons.hpp>

inline void ducode::DesignInstance::write_verilog(const std::string& filename) {
  auto tb_module = std::ranges::find_if(m_design.get_modules(), [&](const auto& mod) { return mod.get_name() == m_design.get_top_module_name(); });
  if (tb_module == m_design.get_modules().end()) {
    throw std::runtime_error("This design does not contain a top module");
  }

  auto top_module = tb_module;
  if (tb_module->get_ports().empty()) {// this is a testbench

    if (tb_module->get_cells().size() > 1) {
      throw std::runtime_error("Here we assume that the testbench contains only 1 instantiated design");
    }
    if (!(tb_module->get_cells().begin()->is_module())) {
      throw std::runtime_error("The first design contained in the testbench is not a module");
    }

    top_module = std::ranges::find_if(m_design.get_modules(), [&](const auto& mod) { return mod.get_name() == tb_module->get_cells().begin()->get_type(); });
  }

  nlohmann::json params;
  params["ift"] = false;
  params["identifier_context"] = "";

  std::stringstream result;
  result << top_module->export_verilog_header(params);

  boost::graph_traits<instantiation_graph>::vertex_iterator vi;
  boost::graph_traits<instantiation_graph>::vertex_iterator vi_end;
  for (boost::tie(vi, vi_end) = boost::vertices(m_graph); vi != vi_end; ++vi) {
    if (m_graph[*vi].type == "module_port") {
      std::string name = fmt::format("{}_{}", boost::algorithm::join(m_graph[*vi].hierarchy, "_"), m_graph[*vi].name);
      const std::string width = m_graph[*vi].port_ptr->m_bits.size() > 1 ? fmt::format("[{}:0] ", m_graph[*vi].port_ptr->m_bits.size() - 1) : "";
      result << fmt::format("wire {}{};\n", width, name);
      if (m_graph[*vi].hierarchy.size() == 1) {
        if (m_graph[*vi].port_ptr->m_direction == ducode::Port::Direction::input) {
          result << fmt::format("assign {} = {};\n", name, m_graph[*vi].port_ptr->m_name);
        } else {
          result << fmt::format("assign {} = {};\n", m_graph[*vi].port_ptr->m_name, name);
        }
      }
      // assign eventual constant bits
      if (m_graph[*vi].port_ptr->contains_constant()) {
        for (const auto& [index, bit]: ranges::views::enumerate(m_graph[*vi].port_ptr->m_bits)) {
          if (bit.is_constant()) {
            const std::string index_string = m_graph[*vi].port_ptr->m_bits.size() > 1 ? fmt::format("[{}]", index) : "";
            result << fmt::format("assign {}{} = 1'b{};\n", name, index_string, bit);
          }
        }
      }
    } else {
      auto parent_module = std::ranges::find_if(m_design.get_modules(), [&](const auto& mod) { return mod.get_name() == m_graph[*vi].parent_module; });
      if (parent_module == m_design.get_modules().end()) {
        throw std::runtime_error("parent module not found");
      }
      auto current_cell = std::ranges::find_if(parent_module->get_cells(), [&](const Cell& cell) { return cell.get_type() == m_graph[*vi].type && cell.get_name() == m_graph[*vi].name; });
      std::string identifier_context = boost::algorithm::join(m_graph[*vi].hierarchy, "_");
      params["identifier_context"] = identifier_context;
      std::unordered_map<std::size_t, std::string> identifiers_map;
      for (const Port& port: current_cell->get_ports()) {
        std::string net_type = "wire";
        if (port.m_direction == Port::Direction::output && current_cell->has_register()) {
          net_type = "reg";
        }
        std::string width = port.m_bits.size() > 1 ? fmt::format("[{}:0]", port.m_bits.size() - 1) : "";
        result << fmt::format("  {} {} {}{};\n", net_type, width, identifier_context, port.m_identifier);
        identifiers_map.emplace(port.m_bits_signature, fmt::format("{}{}", identifier_context, port.m_identifier));
      }
      auto cell_params = params;
      cell_params["identifiers_map"] = identifiers_map;
      result << fmt::format("//{} -> {}\n", m_graph[*vi].parent_module, m_graph[*vi].type);
      if (current_cell != parent_module->get_cells().end()) {
        result << current_cell->export_verilog(cell_params);
      }
      // assign constants
      for (const auto& port: current_cell->get_ports()) {
        if (port.contains_constant()) {
          for (const auto& [index, value]: ranges::views::enumerate(port.m_bits)) {
            if (value.is_constant()) {
              const std::string index_string = port.m_bits.size() > 1 ? fmt::format("[{}]", index) : "";
              result << fmt::format("assign {}{}{} = 1'b{};\n", identifier_context, port.m_identifier, index_string, value);
            }
          }
        }
      }
    }
  }

  result << "initial begin\n";

  std::unordered_set<const Net*> initialized_nets;

  boost::graph_traits<instantiation_graph>::edge_iterator ei;
  boost::graph_traits<instantiation_graph>::edge_iterator ei_end;
  for (boost::tie(ei, ei_end) = boost::edges(m_graph); ei != ei_end; ++ei) {
    auto source_vertex = boost::source(*ei, m_graph);
    if (!initialized_nets.contains(m_graph[*ei].net_ptr) && m_graph[*ei].net_ptr != nullptr && !m_graph[*ei].net_ptr->get_init_value().empty()) {
      initialized_nets.emplace(m_graph[*ei].net_ptr);
      result << fmt::format(" {}{} = {};\n", boost::algorithm::join(m_graph[source_vertex].hierarchy, "_"), m_graph[*ei].source_port_ptr->m_identifier, m_graph[*ei].net_ptr->get_init_value());
    }
  }

  result << "end\n";

  for (boost::tie(ei, ei_end) = boost::edges(m_graph); ei != ei_end; ++ei) {
    auto source_vertex = boost::source(*ei, m_graph);
    auto target_vertex = boost::target(*ei, m_graph);

    std::string source_wire;
    if (m_graph[source_vertex].type == "module_port") {
      source_wire = fmt::format("{}_{}", boost::algorithm::join(m_graph[source_vertex].hierarchy, "_"), m_graph[source_vertex].name);
    } else {
      source_wire = fmt::format("{}{}", boost::algorithm::join(m_graph[source_vertex].hierarchy, "_"), m_graph[*ei].source_port_ptr->m_identifier);
    }

    std::string target_wire;
    if (m_graph[target_vertex].type == "module_port") {
      target_wire = fmt::format("{}_{}", boost::algorithm::join(m_graph[target_vertex].hierarchy, "_"), m_graph[target_vertex].name);
    } else {
      target_wire = fmt::format("{}{}", boost::algorithm::join(m_graph[target_vertex].hierarchy, "_"), m_graph[*ei].target_port_ptr->m_identifier);
    }

    result << fmt::format("//edge {} -> {}\n", m_graph[source_vertex].name, m_graph[target_vertex].name);

    // here we have to solve the inconsistencies between the bit numbers of the ports and the bit numbers of the cells

    const auto* source_bits = &(m_graph[*ei].source_port_ptr->m_bits);
    if (m_graph[*ei].source_port_ptr->m_direction == ducode::Port::Direction::output && m_graph[boost::source(*ei, m_graph)].type == "module_port") {
      const auto& port_it = std::ranges::find_if(m_graph[boost::source(*ei, m_graph)].cell_ptr->get_ports(),
                                                 [&](const auto& port) {
                                                   return port.m_name == m_graph[*ei].source_port_ptr->m_name;
                                                 });
      Expects(port_it != m_graph[boost::source(*ei, m_graph)].cell_ptr->get_ports().end());
      source_bits = &(port_it->m_bits);
    }

    const auto* target_bits = &(m_graph[*ei].target_port_ptr->m_bits);
    if (m_graph[*ei].target_port_ptr->m_direction == ducode::Port::Direction::input && m_graph[boost::target(*ei, m_graph)].type == "module_port") {
      const auto& port_it = std::ranges::find_if(m_graph[boost::target(*ei, m_graph)].cell_ptr->get_ports(),
                                                 [&](const auto& port) {
                                                   return port.m_name == m_graph[*ei].target_port_ptr->m_name;
                                                 });
      target_bits = &(port_it->m_bits);
    }

    if (*target_bits == *source_bits) {
      result << fmt::format("assign {} = {};\n", target_wire, source_wire);
    } else {
      for (const auto& [source_index, source_bit]: ranges::views::enumerate(*source_bits)) {
        if (!source_bit.is_constant()) {
          for (const auto& [target_index, target_bit]: ranges::views::enumerate(*target_bits)) {
            if (!target_bit.is_constant()) {
              if (source_bit == target_bit) {
                std::string source_index_string = m_graph[*ei].source_port_ptr->m_bits.size() > 1 ? fmt::format("[{}]", source_index) : "";
                std::string target_index_string = m_graph[*ei].target_port_ptr->m_bits.size() > 1 ? fmt::format("[{}]", target_index) : "";
                result << fmt::format("assign {}{} = {}{};\n", target_wire, target_index_string, source_wire, source_index_string);
              }
            }
          }
        }
      }
    }
  }

  result << "endmodule\n";

  auto file = std::ofstream(filename, std::ofstream::out);
  file << result.str();
}

/*
 * parameters:
 *  - auto initial_edge: starting condition
 *  - auto edge_comparison and auto edge_selector: Predecessor/Successor selection condition
 *  - auto result_type: single path, multiple paths or full subcircuit
 *  - auto search_direction: forward, backward or both
 */
inline std::deque<ducode::DesignInstance::instantiation_graph::vertex_descriptor> ducode::DesignInstance::get_path(const auto& initial_edge, const auto& edge_comparison, const auto& edge_filter, const auto& edge_selector, const std::string& search_direction, const std::string& result_type) const {

  std::deque<ducode::DesignInstance::instantiation_graph::vertex_descriptor> path;
  std::deque<ducode::DesignInstance::instantiation_graph::vertex_descriptor> visited;

  path.emplace_back(initial_edge->m_source);
  path.emplace_back(initial_edge->m_target);
  visited.emplace_back(initial_edge->m_source);
  visited.emplace_back(initial_edge->m_target);

  auto source_vertex = m_graph.m_vertices[initial_edge->m_source];

  if (search_direction == "backwards" || search_direction == "both") {
    // backward direction
    while (!(source_vertex.m_in_edges.empty() && (source_vertex.m_property.port_ptr != nullptr ? source_vertex.m_property.port_ptr->m_direction == Port::Direction::input : false))) {
      auto visited_filter = [&](const auto& edge) { return !ducode::contains(visited, edge.get_target()); };
      auto in_edges_filtered = source_vertex.m_in_edges | std::ranges::views::filter(edge_filter) | std::ranges::views::filter(visited_filter);
      if (std::distance(in_edges_filtered.begin(), in_edges_filtered.end()) == 0 && result_type == "path") {
        if (path.size() == 1 || path.front() == initial_edge->m_source) {// path can only backtrack to the initial element of the path
          break;
        }
        path.pop_front();
        source_vertex = m_graph.m_vertices[path.front()];
        continue;/// if in_edges_filtered is empty then break
      }
      auto edge = edge_selector(in_edges_filtered, edge_comparison);
      // do i need this path check now that i have the visited vector????
      //      if (ducode::contains(path, edge->get_target()) && result_type == "path") {
      //        path.pop_front();
      //        source_vertex = m_graph.m_vertices[path.front()];
      //        continue;;
      //      }
      path.emplace_front(edge->get_target());
      visited.emplace_front(edge->get_target());
      source_vertex = m_graph.m_vertices[edge->get_target()];
    }
  }

  if (search_direction == "forwards" || search_direction == "both") {
    source_vertex = m_graph.m_vertices[initial_edge->m_target];

    // forward direction
    while (!(source_vertex.m_out_edges.empty() && (source_vertex.m_property.port_ptr != nullptr ? source_vertex.m_property.port_ptr->m_direction == Port::Direction::output : false))) {
      auto visited_filter = [&](const auto& edge) { return !ducode::contains(visited, edge.get_target()); };
      auto out_edges_filtered = source_vertex.m_out_edges | std::ranges::views::filter(edge_filter) | std::ranges::views::filter(visited_filter);
      if (std::distance(out_edges_filtered.begin(), out_edges_filtered.end()) == 0 && result_type == "path") {
        if (path.size() == 1 || path.back() == initial_edge->m_target) {// path can only backtrack to the initial element of the path
          break;
        }
        path.pop_back();
        source_vertex = m_graph.m_vertices[path.back()];
        continue;
      }
      auto edge = edge_selector(out_edges_filtered, edge_comparison);

      path.emplace_back(edge->get_target());
      visited.emplace_back(edge->get_target());
      source_vertex = m_graph.m_vertices[edge->get_target()];
    }
  }

  return path;
}

inline auto ducode::DesignInstance::find_tag_in_edge(auto& all_edges_filtered, auto& input_tag, auto& vcd_nr) const {
  for (auto& edge: all_edges_filtered) {
    auto tag_vector = get_tag_values(edge).at(vcd_nr);
    for (auto& tag_values: *tag_vector) {
      if (tag_values.value == input_tag) {
        return edge;
      }
    }
  }
}

inline bool ducode::DesignInstance::bitwise_vcd_compare(VCDBitVector vec1, VCDBitVector vec2) {
  VCDBitVector result_vec;
  for (auto i = 0ul; i < vec1.size(); i++) {
    if (vec1[i] == vec2[i] && vec1[i] == VCDBit::VCD_1 && vec2[i] == VCDBit::VCD_1) {
      result_vec.emplace_back(VCDBit::VCD_1);
    } else {
      result_vec.emplace_back(VCDBit::VCD_0);
    }
  }
  return std::ranges::any_of(result_vec, [](auto bit) { return bit == VCDBit::VCD_1; });
}

/// count the number of appearances of 'elements' in all paths
inline uint32_t count_element_occurrences(std::vector<std::deque<ducode::DesignInstance::instantiation_graph::vertex_descriptor>> paths, std::deque<ducode::DesignInstance::instantiation_graph::vertex_descriptor> elements) {
  uint32_t result = 0;
  bool contains = true;
  for (auto& path: paths) {
    if (std::find(path.begin(), path.end(), elements.at(0)) != path.end()) {
      auto it_path = std::find(path.begin(), path.end(), elements.at(0));
      auto it_elements = elements.begin();
      for (uint32_t i = 0; i < elements.size(); i++) {
        if (it_elements == elements.end() && it_path != path.end()) {
          break;
        }
        if (it_elements != elements.end() && it_path == path.end()) {
          contains = false;
          break;
        }
        if (*it_elements != *it_path) {
          contains = false;
          break;
        }
        it_path++;
        it_elements++;
      }
    }
    if (contains) {
      result++;
    }
  }
  return result;
}

inline std::vector<std::pair<std::deque<ducode::DesignInstance::instantiation_graph::vertex_descriptor>, uint32_t>> ducode::DesignInstance::count_common_path_elements(std::vector<std::deque<ducode::DesignInstance::instantiation_graph::vertex_descriptor>>& paths) {
  std::deque<ducode::DesignInstance::instantiation_graph::vertex_descriptor> elements;
  std::vector<std::pair<std::deque<ducode::DesignInstance::instantiation_graph::vertex_descriptor>, uint32_t>> result;
  for (const auto& path: paths) {
    for (const auto& [index, element]: ranges::views::enumerate(path)) {
      elements.clear();
      //      elements.emplace_back(element);
      for (auto i = index; i < path.size(); i++) {
        elements.emplace_back(path.at(i));
        if (!std::ranges::any_of(result, [&](auto ele) { return ele.first == elements; })) {
          result.emplace_back(elements, count_element_occurrences(paths, elements));
        }
      }
    }
  }
  return result;
}

inline std::vector<std::pair<std::string, uint32_t>> ducode::DesignInstance::count_output_tags() const {
  std::vector<std::pair<std::string, uint32_t>> result;
  auto top_module_outputs = m_design.get_module(m_design.get_top_module_name()).get_output_names();
  auto vertex_filter = [&](const auto& vertex) { if (vertex.m_property.port_ptr != nullptr) { return ((vertex.m_property.port_ptr->m_direction == ducode::Port::Direction::output) && (std::find(top_module_outputs.begin(), top_module_outputs.end(), vertex.m_property.port_ptr->m_name) != top_module_outputs.end()) && (vertex.m_property.hierarchy.size() == 1) ); } return false; };
  uint32_t count = 0;
  auto all_output_vertices = m_graph.m_vertices | std::ranges::views::filter(vertex_filter);
  assert(!all_output_vertices.empty());

  for (const auto& vertex: all_output_vertices) {
    count = 0;
    std::vector<VCDSignalHash> edge_hashes;
    if (vertex.m_in_edges.empty()) { continue; }
    std::vector<const std::deque<VCDTimedValue>*> tag_vector = get_tag_values(vertex.m_in_edges[0]);

    for (auto& tag_values: tag_vector) {
      for (const auto& tags: *tag_values) {
        for (auto& bit: tags.value.get_value_vector()) {
          if (bit == VCDBit::VCD_1) {
            count++;
          }
        }
      }
    }
    result.emplace_back(vertex.m_property.name, count);
  }
  return result;
}

inline void ducode::DesignInstance::add_vcd_data(const std::shared_ptr<VCDFile>& vcd_data) {

  auto tb_module = std::ranges::find_if(m_design.get_modules(), [&](const auto& mod) { return mod.get_name() == m_design.get_top_module_name(); });
  if (tb_module == m_design.get_modules().end()) {
    throw std::runtime_error("This design does not contain a top module");
  }

  auto top_module = tb_module;
  if (tb_module->get_ports().empty()) {// this is a testbench

    if (tb_module->get_cells().size() > 1) {
      throw std::runtime_error("Here we assume that the testbench contains only 1 instantiated design");
    }
    if (!(tb_module->get_cells().begin()->is_module())) {
      throw std::runtime_error("The first design contained in the testbench is not a module");
    }

    top_module = std::ranges::find_if(m_design.get_modules(), [&](const auto& mod) { return mod.get_name() == tb_module->get_cells().begin()->get_type(); });
  }

  if (!m_vcd_data.empty()) {
    auto* tb_scope_new = vcd_data->root_scope->children[0];
    auto* outer_scope = *(tb_scope_new->children.begin());

    auto* tb_scope_reference = m_vcd_data.back()->root_scope->children[0];
    const auto& outer_scope_reference = *(tb_scope_reference->children.begin());
    for (const auto& port: top_module->get_ports()) {
      auto lambda = [&](const auto& signal) { return signal->reference == port.m_name; };
      auto hash_iter = std::ranges::find_if(outer_scope->signals, lambda);
      auto hash_reference_iter = std::ranges::find_if(outer_scope_reference->signals, lambda);
      Expects(hash_iter != outer_scope->signals.end());
      Expects(hash_reference_iter != outer_scope_reference->signals.end());

      const auto& signal_values = vcd_data->get_signal_values((*hash_iter)->hash);
      const auto& signal_values_reference = m_vcd_data.back()->get_signal_values((*hash_reference_iter)->hash);
      if (signal_values != signal_values_reference) {
        throw std::runtime_error("the VCD file is part of a different simulation");
      }
    }
  }

  m_vcd_data.emplace_back(vcd_data);

  Expects(m_vcd_data.back()->root_scope != nullptr);
  Expects(!m_vcd_data.back()->root_scope->children.empty());
  auto* tb_scope = m_vcd_data.back()->root_scope->children[0];
  Expects(tb_scope != nullptr);
  Expects(!tb_scope->children.empty());
  auto* top_module_scope = *(tb_scope->children.begin());

  auto get_scope_hierarchy = [](const VCDScope& scope) -> std::vector<std::string> {
    std::deque<std::string> result;
    const VCDScope* temp = &scope;
    while (temp != nullptr) {
      result.emplace_front(temp->name);
      temp = temp->parent;
    }
    return {result.begin() + 2, result.end()};
  };

  auto compare_hierarchies = [](const auto& hierarchy1, const auto& hierarchy2) {
    auto drop1 = hierarchy1 | std::ranges::views::drop(1);
    auto drop2 = hierarchy2 | std::ranges::views::drop(1);

    return std::ranges::equal(drop1, drop2);
  };

  boost::graph_traits<instantiation_graph>::edge_iterator ei;
  boost::graph_traits<instantiation_graph>::edge_iterator ei_end;
  for (boost::tie(ei, ei_end) = boost::edges(m_graph); ei != ei_end; ++ei) {
    if (m_graph[*ei].net_ptr != nullptr) {
      const VCDScope* selected_scope = nullptr;
      if (m_graph[*ei].hierarchy.size() == 1) {
        selected_scope = top_module_scope;
      } else {
        auto matching_scopes = m_vcd_data.back()->get_scopes() | std::ranges::views::filter([&](const auto& scope) { return scope.name == legalize_identifier(m_graph[*ei].hierarchy.back()); });
        auto matching_scopes_size = ranges::count_if(matching_scopes, [](const auto&) { return true; });
        if (matching_scopes_size == 1) {
          selected_scope = &*matching_scopes.begin();
        } else if (m_graph[*ei].hierarchy.size() > 2) {
          const auto scope_it = std::ranges::find_if(matching_scopes,
                                                     [&](const auto& scope) {
                                                       return compare_hierarchies(get_scope_hierarchy(scope), m_graph[*ei].hierarchy);
                                                     });
          if (scope_it == matching_scopes.end()) {
            spdlog::info("{}", m_graph[*ei].hierarchy);
            throw std::runtime_error("Scope not found.");
          }
          selected_scope = &*scope_it;
        } else {
          throw std::runtime_error("This is a strange case. Not managed for now.");
        }
      }

      auto net_name_legalized = legalize_identifier(m_graph[*ei].net_ptr->get_name());
      auto signal_it = std::ranges::find_if(selected_scope->signals, [&](const VCDSignal* signal) { return signal->reference == net_name_legalized; });
      if (signal_it == selected_scope->signals.end()) {
        throw std::runtime_error(fmt::format("The referenced net {} was not found in the scope {}", m_graph[*ei].net_ptr->get_name(), selected_scope->name));
      }
      m_graph[*ei].value_signal = *signal_it;

      auto tags_signal_it = std::ranges::find_if(selected_scope->signals, [&](const VCDSignal* signal) { return signal->reference == net_name_legalized + "_t"; });
      if (tags_signal_it != selected_scope->signals.end()) {
        m_graph[*ei].tags_signal.emplace_back(*tags_signal_it);
      }
    }
  }
}

struct PairHash {
  template<typename T, typename U>
  size_t operator()(const std::pair<T, U>& value) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, value.first);
    boost::hash_combine(seed, value.second);
    return seed;
  }
};

inline void ducode::DesignInstance::populate_module(instantiation_graph::vertex_descriptor module_vertex, std::vector<std::string> hierarchy, const Cell& cell_module) {
  Expects(cell_module.is_module());
  Expects(cell_module.get_name() == m_graph[module_vertex].name);

  std::vector<instantiation_graph::vertex_descriptor> new_cell_vertices = create_module_ports_vertices(module_vertex, hierarchy, cell_module);

  auto module_it = std::ranges::find_if(m_design.get_modules(), [&](const auto& mod) { return cell_module.get_type() == mod.get_name(); });

  if (module_it == m_design.get_modules().end()) {
    throw std::runtime_error("module not found");
  }

  new_cell_vertices.reserve(new_cell_vertices.size() + module_it->get_cells().size());

  hierarchy.emplace_back(cell_module.get_name());

  for (const auto& cell: module_it->get_cells()) {
    VertexInfo module_vertex_prop;
    module_vertex_prop.name = cell.get_name();
    module_vertex_prop.parent_module = module_it->get_name();
    module_vertex_prop.hierarchy = hierarchy;
    module_vertex_prop.type = cell.get_type();
    module_vertex_prop.cell_ptr = &cell;
    new_cell_vertices.emplace_back(boost::add_vertex(module_vertex_prop, m_graph));
  }

  std::unordered_map<ducode::Bit, std::unordered_set<std::pair<instantiation_graph::vertex_descriptor, const ducode::Port*>, PairHash>> bit_to_target_vertices;

  const auto insert_into_mapset = [&](ducode::Bit bit, instantiation_graph::vertex_descriptor vertex, const ducode::Port* port) {
    const auto it = bit_to_target_vertices.find(bit);
    if (it != bit_to_target_vertices.end()) {
      it->second.emplace(vertex, port);
    } else {
      bit_to_target_vertices.emplace(bit, std::unordered_set<std::pair<instantiation_graph::vertex_descriptor, const ducode::Port*>, PairHash>{{vertex, port}});
    }
  };

  for (const auto& new_cell_vertex: new_cell_vertices) {
    if (m_graph[new_cell_vertex].type == "module_port") {
      if (m_graph[new_cell_vertex].port_ptr->m_direction == ducode::Port::Direction::output) {
        for (const auto& bit: m_graph[new_cell_vertex].port_ptr->m_bits) {
          if (!bit.is_constant()) {
            insert_into_mapset(bit, new_cell_vertex, m_graph[new_cell_vertex].port_ptr);
          }
        }
      }
    } else {
      for (const auto& port: m_graph[new_cell_vertex].cell_ptr->get_ports() | std::ranges::views::filter([](const auto& port) { return port.m_direction == ducode::Port::Direction::input; })) {
        for (const auto& bit: port.m_bits) {
          if (!bit.is_constant()) {
            insert_into_mapset(bit, new_cell_vertex, &port);
          }
        }
      }
    }
  }

  std::vector<instantiation_graph::edge_descriptor> new_edges;

  for (const auto& new_cell_vertex: new_cell_vertices) {

    const auto connect_port = [&](const auto& port) {
      std::unordered_set<std::string> connected_ports;
      for (const auto& bit: port.m_bits) {
        const auto target_vertex_it = bit_to_target_vertices.find(bit);
        if (target_vertex_it == bit_to_target_vertices.end()) {
          continue;
        }
        for (const auto& target_vertex: target_vertex_it->second) {
          if (!connected_ports.contains(target_vertex.second->m_identifier)) {
            connected_ports.emplace(target_vertex.second->m_identifier);
            auto edge = add_edge(new_cell_vertex, target_vertex.first, &port, target_vertex.second);
            m_graph[edge].parent_module = module_it->get_name();
            m_graph[edge].hierarchy = hierarchy;
            new_edges.emplace_back(edge);
          }
        }
      }
    };

    if (m_graph[new_cell_vertex].type == "module_port") {
      if (m_graph[new_cell_vertex].port_ptr->m_direction == ducode::Port::Direction::input) {
        connect_port(*m_graph[new_cell_vertex].port_ptr);
      }
    } else {
      for (const auto& port: m_graph[new_cell_vertex].cell_ptr->get_ports() | std::ranges::views::filter([](const auto& port) { return port.m_direction == ducode::Port::Direction::output; })) {
        connect_port(port);
      }
    }
  }

  boost::clear_vertex(module_vertex, m_graph);
  boost::remove_vertex(module_vertex, m_graph);

  // associate nets to edges
  const auto& nets = m_design.get_module(cell_module.get_type()).get_nets();
  std::unordered_map<std::size_t, const Net*> signature_to_net;

  for (const auto& net: nets) {
    signature_to_net.emplace(net.get_bits_signature(), &net);
  }

  for (const auto& edge: new_edges) {
    if (signature_to_net.contains(m_graph[edge].source_port_ptr->m_bits_signature)) {
      m_graph[edge].net_ptr = signature_to_net.at(m_graph[edge].source_port_ptr->m_bits_signature);
    } else if (signature_to_net.contains(m_graph[edge].target_port_ptr->m_bits_signature)) {
      m_graph[edge].net_ptr = signature_to_net.at(m_graph[edge].target_port_ptr->m_bits_signature);
    }
  }

  for (const auto& cell: module_it->get_cells()) {
    if (cell.is_module()) {
      for (auto i = 0UL; i < m_graph.m_vertices.size(); ++i) {
        if (m_graph.m_vertices[i].m_property.hierarchy == hierarchy && m_graph.m_vertices[i].m_property.name == cell.get_name()) {
          populate_module(i, hierarchy, cell);
        }
      }
    }
  }
}

inline std::vector<ducode::DesignInstance::instantiation_graph::vertex_descriptor> ducode::DesignInstance::create_module_ports_vertices(instantiation_graph::vertex_descriptor module_vertex, std::vector<std::string> hierarchy, const Cell& cell_module) {
  Expects(cell_module.is_module());
  Expects(cell_module.get_name() == m_graph[module_vertex].name);

  hierarchy.emplace_back(cell_module.get_name());

  auto module_it = std::ranges::find_if(m_design.get_modules(), [&](const auto& mod) { return cell_module.get_type() == mod.get_name(); });

  if (module_it == m_design.get_modules().end()) {
    throw std::runtime_error("module not found");
  }

  std::vector<instantiation_graph::vertex_descriptor> new_cell_vertices;

  for (const auto& port: module_it->get_ports()) {
    VertexInfo module_vertex_prop;
    module_vertex_prop.name = fmt::format("{}_{}", module_it->get_name(), port.m_name);
    module_vertex_prop.parent_module = module_it->get_name();
    module_vertex_prop.hierarchy = hierarchy;
    module_vertex_prop.type = "module_port";
    module_vertex_prop.port_ptr = &port;
    module_vertex_prop.cell_ptr = &cell_module;
    new_cell_vertices.emplace_back(boost::add_vertex(module_vertex_prop, m_graph));
  }

  // connect newly added nodes to the previously present nodes

  for (auto [edge_in, edge_in_end] = boost::in_edges(module_vertex, m_graph); edge_in != edge_in_end; ++edge_in) {
    const auto& source_vertex = boost::source(*edge_in, m_graph);
    for (const auto target_vertex: new_cell_vertices) {
      if (m_graph[*edge_in].target_port_ptr->m_name == m_graph[target_vertex].port_ptr->m_name) {
        const auto& edge_descriptor = add_edge(source_vertex, target_vertex, m_graph[*edge_in].source_port_ptr, m_graph[target_vertex].port_ptr);
        m_graph[edge_descriptor] = m_graph[*edge_in];
      }
    }
  }

  for (auto [edge_out, edge_out_end] = boost::out_edges(module_vertex, m_graph); edge_out != edge_out_end; ++edge_out) {
    const auto& target_vertex = boost::target(*edge_out, m_graph);
    for (const auto source_vertex: new_cell_vertices) {
      if (m_graph[*edge_out].source_port_ptr->m_name == m_graph[source_vertex].port_ptr->m_name) {
        const auto& edge_descriptor = add_edge(source_vertex, target_vertex, m_graph[source_vertex].port_ptr, m_graph[*edge_out].target_port_ptr);
        m_graph[edge_descriptor] = m_graph[*edge_out];
      }
    }
  }

  return new_cell_vertices;
}

inline ducode::DesignInstance::instantiation_graph::edge_descriptor ducode::DesignInstance::add_edge(const auto& source_vertex, const auto& target_vertex, const ducode::Port* source_port_ptr, const ducode::Port* target_port_ptr) {
  auto edge = boost::add_edge(source_vertex, target_vertex, m_graph).first;

  const std::string name = fmt::format("{} -> {}", source_port_ptr->m_bits, target_port_ptr->m_bits);

  spdlog::debug("Adding edge from {} to {}", source_port_ptr->m_identifier, target_port_ptr->m_identifier);

  m_graph[edge].name = name;
  m_graph[edge].source_port_ptr = source_port_ptr;
  m_graph[edge].target_port_ptr = target_port_ptr;

  return edge;
}

inline void ducode::DesignInstance::propagate_net_pointers_impl(const ducode::DesignInstance::instantiation_graph::edge_descriptor& current_edge, const ducode::Net* previous_edge_net, std::unordered_set<ducode::DesignInstance::instantiation_graph::edge_descriptor>& visited) {
  if (visited.contains(current_edge)) {
    return;
  }

  visited.emplace(current_edge);

  if (m_graph[current_edge].net_ptr == nullptr) {
    m_graph[current_edge].net_ptr = previous_edge_net;
    spdlog::info("Net propagated to {}", m_graph[current_edge].name);
  }

  const auto& source = boost::source(current_edge, m_graph);
  for (const auto& edge: subrange(boost::in_edges(source, m_graph))) {
    propagate_net_pointers_impl(edge, m_graph[current_edge].net_ptr, visited);
  }
}

inline void ducode::DesignInstance::propagate_net_pointers(const ducode::Module& top_module) {
  std::unordered_set<ducode::DesignInstance::instantiation_graph::edge_descriptor> visited;

  for (const auto& vertex: ducode::subrange(boost::vertices(m_graph))) {
    for (const auto& port: top_module.get_ports()) {
      if (m_graph[vertex].type == "module_port" && m_graph[vertex].name == fmt::format("{}_{}", top_module.get_name(), port.m_name)) {
        spdlog::info("Starting propagation from {}", port.m_name);

        const auto net_it = std::ranges::find_if(top_module.get_nets(), [&](const ducode::Net& net) { return net.get_name() == port.m_name; });
        if (net_it == top_module.get_nets().end()) {
          throw std::runtime_error(fmt::format("Net {} not found", port.m_name));
        }

        for (const auto& edge: subrange(boost::in_edges(vertex, m_graph))) {
          propagate_net_pointers_impl(edge, &(*net_it), visited);
        }
      }
    }
  }
}
