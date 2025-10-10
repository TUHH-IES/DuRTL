/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/bit.hpp>
#include <ducode/cell.hpp>
#include <ducode/cell/cell_module.hpp>
#include <ducode/design.hpp>
#include <ducode/instantiation_graph_traits.hpp>
#include <ducode/module.hpp>
#include <ducode/net.hpp>
#include <ducode/port.hpp>
#include <ducode/signals_data_manager.hpp>
#include <ducode/types.hpp>
#include <ducode/utility/concepts.hpp>
#include <ducode/utility/contains.hpp>
#include <ducode/utility/subrange.hpp>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/container_hash/hash.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/detail/adjacency_list.hpp>
#include <boost/graph/graph_selectors.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graphviz.hpp>
#include <fmt/core.h>
#include <gsl/assert>
#include <gsl/narrow>
#include <z3++.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace ducode {

class DesignInstance {
public:
  instantiation_graph m_graph;
  Design m_design;
  std::shared_ptr<SignalsDataManager> m_signal_tag_map;
  std::unique_ptr<CellModule> m_root_cell_module;

protected:
  instantiation_graph::edge_descriptor add_edge(const instantiation_graph::vertex_descriptor& source_vertex, const instantiation_graph::vertex_descriptor& target_vertex, const Port* source_port_ptr, const Port* target_port_ptr);
  std::vector<instantiation_graph::vertex_descriptor> create_module_ports_vertices(instantiation_graph::vertex_descriptor module_vertex, std::vector<std::string> hierarchy, const Cell& cell_module);
  void populate_module(instantiation_graph::vertex_descriptor module_vertex, std::vector<std::string> hierarchy, const Cell& cell_module);
  void propagate_net_pointers(const Module& top_module);
  void propagate_net_pointers_impl(const instantiation_graph::edge_descriptor& current_edge, const Net* previous_edge_net, std::unordered_set<instantiation_graph::edge_descriptor>& visited);

public:
  static DesignInstance create_instance(const Design& design) {
    DesignInstance inst;
    inst.m_design = design;

    auto tb_module = std::ranges::find_if(inst.m_design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });
    if (tb_module == inst.m_design.get_modules().end()) {
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

      top_module = std::ranges::find_if(inst.m_design.get_modules(), [&](const auto& mod) { return mod.get_name() == tb_module->get_cells().begin()->get_type(); });
    }

    const std::vector<std::string> hierarchy;

    std::vector<Port> ports;
    inst.m_root_cell_module = std::make_unique<CellModule>(top_module->get_name(), top_module->get_name(), ports, false, nlohmann::json{}, nlohmann::json{});

    InstantiationGraphVertexInfo module_vertex_prop;
    module_vertex_prop.name = top_module->get_name();
    module_vertex_prop.parent_module = "top_module";
    module_vertex_prop.hierarchy = hierarchy;
    module_vertex_prop.type = top_module->get_name();
    module_vertex_prop.cell_ptr = inst.m_root_cell_module.get();
    auto module_vertex = boost::add_vertex(module_vertex_prop, inst.m_graph);

    inst.populate_module(module_vertex, hierarchy, *inst.m_root_cell_module);
    inst.propagate_net_pointers(*top_module);

    for (const auto& edge: subrange(boost::edges(inst.m_graph))) {
      if (inst.m_graph[edge].net_ptr == nullptr) {
        throw std::runtime_error(fmt::format("Unrecognized net for edge: {} -> {}", inst.m_graph[edge].source_port_ptr->m_identifier, inst.m_graph[edge].target_port_ptr->m_identifier));
      }
    }

    return inst;
  }

  void add_signal_tag_map(std::shared_ptr<SignalsDataManager> signal_tag_map) {
    m_signal_tag_map = std::move(signal_tag_map);
  }

  [[nodiscard]] TagSourceList get_information_flows_for_output_from_inputs_at_timestep(const std::vector<SignalIdentifier>& /*source_ids*/, SignalIdentifier& /*target_id*/, SignalTime /*timepoint*/) const;
  [[nodiscard]] TagSourceList get_information_flows_for_output_from_inputs(const std::vector<SignalIdentifier>& source_ids, SignalIdentifier& target_id) const;

  [[nodiscard]] auto& get_design() const {
    return m_design;
  }

  //[[nodiscard]] SignalValues get_signal_values(const SignalIdentifier& signal_id) const;

  [[nodiscard]] SignalValues get_signal_values(const instantiation_graph::edge_descriptor& edge) const {
    return m_signal_tag_map->get_signal_values({.name = m_graph[edge].net_ptr->get_name(), .hierarchy = m_graph[edge].hierarchy});
  }


  [[nodiscard]] SignalValue get_signal_value(const instantiation_graph::edge_descriptor& edge, SignalTime timestep) const {
    return m_signal_tag_map->get_signal_value({.name = m_graph[edge].net_ptr->get_name(), .hierarchy = m_graph[edge].hierarchy}, timestep);
  }

  [[nodiscard]] SignalValue get_signal_value(const SignalIdentifier& signal_id, SignalTime timestep) const {
    return m_signal_tag_map->get_signal_value(signal_id, timestep);
  }

  [[nodiscard]] uint32_t number_of_empty_edges() const {
    uint32_t count = 0;

    for (const auto& edge: m_graph.m_edges) {
      if (edge.get_property().net_ptr == nullptr) {
        count++;
      }
    }
    return count;
  }

  // ifa query functions - use cases
  /* [[nodiscard]] std::optional<bool> if_input_to_signal(const instantiation_graph::edge_descriptor& input_edge, SignalTime input_time, const instantiation_graph::edge_descriptor& target_edge, SignalTime target_time) {
    m_signal_value_manager.tag_exists(m_graph[input_edge].get_net().get_name(), input_time, m_graph[target_edge].get_net().get_name(), target_time);

   }
   */
  [[nodiscard]] std::vector<SignalValues> get_tag_values(const instantiation_graph::edge_descriptor& edge) const {
    return m_signal_tag_map->get_tag_values({.name = m_graph[edge].net_ptr->get_name(), .hierarchy = m_graph[edge].hierarchy});
  }

  [[nodiscard]] std::string get_clock_signal_from_dffs() {
    if (!check_clock_signal()) {
      throw std::runtime_error("Clock signal not the same across all flipflops! - Multiple clock signals detected!");
    }

    for (const auto& vertex: subrange(boost::vertices(m_graph))) {
      if (m_graph[vertex].cell_ptr != nullptr) {
        if (m_graph[vertex].cell_ptr->has_register()) {
          ducode::Port clk_port;
          for (const auto& port: m_graph[vertex].cell_ptr->get_ports()) {
            if (port.m_name == "CLK") {
              clk_port = port;
            }
          }
          //find the in_edge that matches the bit signature
          auto [edge_begin, edge_end] = boost::in_edges(vertex, m_graph);
          for (auto edge_it = edge_begin; edge_it != edge_end; ++edge_it) {
            // the clock signal should always be a complete bits_signature
            if (m_graph[*edge_it].net_ptr->get_bits_signature() == clk_port.m_bits_signature) {
              return m_graph[*edge_it].net_ptr->get_name();
              break;
            }
          }
        }
      }
    }
    throw std::runtime_error("No clock signal found!");
  }

  // Check if all flipflops in the design use the same clock signal.
  // IMPORTANT: This function does not work correctly if the design has multiple modules.
  //            Even if all flipflops use the same clock signal, but are in different modules,
  //            the actual nets for each module clock signal are different.
  //            To solve this issue the different clock signal nets would need to be traced back to the top module clk signal.
  [[nodiscard]] bool check_clock_signal() {
    const ducode::Net* clock_net = nullptr;

    for (const auto& vertex: subrange(boost::vertices(m_graph))) {
      if (m_graph[vertex].cell_ptr != nullptr) {
        if (m_graph[vertex].cell_ptr->has_register()) {
          ducode::Port clk_port;
          for (const auto& port: m_graph[vertex].cell_ptr->get_ports()) {
            if (port.m_name == "CLK") {
              clk_port = port;
            }
          }
          //find the in_edge that matches the bit signature
          auto [edge_begin, edge_end] = boost::in_edges(vertex, m_graph);
          for (auto edge_it = edge_begin; edge_it != edge_end; ++edge_it) {
            // the clock signal should always be a complete bits_signature
            if (m_graph[*edge_it].net_ptr->get_bits_signature() == clk_port.m_bits_signature) {
              if (clock_net == nullptr) {
                clock_net = m_graph[*edge_it].net_ptr;
              }
              if (clock_net != m_graph[*edge_it].net_ptr) {
                return false;
              }
              break;
            }
          }
        }
      }
    }
    return true;
  }

  // Check if all flipflops in the design use the positive clock edge.
  [[nodiscard]] bool check_clock_edge() {
    auto clock_edge_check = [&](auto dff) {
      if (m_graph[dff].cell_ptr != nullptr) {
        if (m_graph[dff].cell_ptr->has_register()) {
          std::string clk_edge = m_graph[dff].cell_ptr->get_parameters().at("CLK_POLARITY").template get<std::string>();
          if (clk_edge != "1") {
            return false;
          }
        }
      }
      return true;
    };

    auto [dff_begin, dff_end] = boost::vertices(m_graph);
    return std::ranges::all_of(dff_begin, dff_end, clock_edge_check);
  }


  [[nodiscard]] std::vector<SignalIdentifier>
  get_all_flipflop_ids() const {
    std::vector<SignalIdentifier> result_vector;

    for (const auto& vertex: subrange(boost::vertices(m_graph))) {
      if (m_graph[vertex].cell_ptr != nullptr) {
        if (m_graph[vertex].cell_ptr->has_register()) {
          if (out_degree(vertex, m_graph) >= 1) {
            auto edge = boost::out_edges(vertex, m_graph);
            const SignalIdentifier name = {.name = m_graph[*edge.first].net_ptr->get_name(), .hierarchy = m_graph[*edge.first].hierarchy};
            result_vector.emplace_back(name);
          } else {
            throw std::runtime_error("Unexpected number of output edges.(0)");
          }
        }
      }
    }
    return result_vector;
  }

  static void write_graphviz(const instantiation_graph& graph, const std::string& filename) {
    auto file = std::ofstream(filename, std::ofstream::out);
    boost::write_graphviz(file, graph,
                          boost::make_label_writer(boost::get(&InstantiationGraphVertexInfo::name, graph)),
                          boost::make_label_writer(boost::get(&InstantiationGraphEdgeInfo::name, graph)));
  }

  void write_graphviz(const std::string& filename) {
    auto file = std::ofstream(filename, std::ofstream::out);
    boost::write_graphviz(file, m_graph,
                          boost::make_label_writer(boost::get(&InstantiationGraphVertexInfo::name, m_graph)),
                          boost::make_label_writer(boost::get(&InstantiationGraphEdgeInfo::name, m_graph)));
  }

  static void write_smt2(z3::solver& solver, const std::string& filename) {
    std::stringstream result;
    result << solver.to_smt2();

    std::ofstream file(filename, std::ofstream::out);
    file << result.str();
  }

  void write_verilog(const std::string& filename);

  template<filesystem_like T>
  void write_verilog(const T& filename) {
    return write_verilog(filename.string());
  }

  [[nodiscard]] static auto vectors_intersection(const fast_bit_container& bits1, const fast_bit_container& bits2) {
    if (bits1.empty() || bits2.empty()) {
      throw std::runtime_error("Empty bits!");
    }

    return std::ranges::any_of(bits1, [&](const Bit& bit) { return bits2.contains(bit); });
  };
  void set_smt_signal_module_port_successor_connections(z3::context& ctx, uint32_t design_instance_id, const z3::expr_vector& ev_edges, const std::unordered_map<std::string, uint32_t>& signal_name_to_edge_id, std::unordered_map<std::string, z3::expr>& port_expr_map, const instantiation_graph::vertex_descriptor& vertex, SignalTime unroll_factor);
  void set_smt_signal_cell_successor_connections(z3::context& ctx, uint32_t design_instance_id, const z3::expr_vector& ev_edges, const std::unordered_map<std::string, uint32_t>& signal_name_to_edge_id, std::unordered_map<std::string, z3::expr>& port_expr_map, const instantiation_graph::vertex_descriptor& vertex, SignalTime unroll_factor, bool dff_enable = false);
  void set_smt_signal_cell_predecessor_connections(z3::context& ctx, uint32_t design_instance_id, const z3::expr_vector& ev_edges, const std::unordered_map<std::string, uint32_t>& signal_name_to_edge_id, std::unordered_map<std::string, z3::expr>& port_map, const instantiation_graph::vertex_descriptor& vertex, SignalTime unroll_factor);
  void set_smt_signal_module_port_predecessor_connections(z3::context& ctx, uint32_t design_instance_id, const z3::expr_vector& ev_edges, const std::unordered_map<std::string, uint32_t>& signal_name_to_edge_id, std::unordered_map<std::string, z3::expr>& port_map, const instantiation_graph::vertex_descriptor& vertex, SignalTime unroll_factor);

  void export_smt2(z3::context& ctx, z3::solver& solver, z3::expr_vector& ev_edges, std::unordered_map<std::string, uint32_t>& signal_name_to_edge_id, uint32_t design_instance_id = 1, SignalTime unroll_factor = 0);

  [[nodiscard]] std::deque<instantiation_graph::vertex_descriptor> get_path(const instantiation_graph::edge_descriptor& initial_edge, const auto& edge_comparison, const auto& edge_filter, const auto& edge_selector, const std::string& search_direction, const std::string& result_type) const;

  [[nodiscard]] std::vector<std::pair<std::string, uint32_t>> count_output_tags() const;

  [[nodiscard]] static std::vector<std::pair<std::deque<instantiation_graph::vertex_descriptor>, uint32_t>> count_common_path_elements(std::vector<std::deque<instantiation_graph::vertex_descriptor>>& paths);

  [[nodiscard]] static bool bitwise_signal_compare(SignalBitVector vec1, SignalBitVector vec2);

  [[nodiscard]] uint32_t tag_cnt_single_edge(std::string& edge_name) const;
  [[nodiscard]] uint32_t tag_cnt_multiple_edges(std::vector<std::string>& edge_names) const;
  [[nodiscard]] uint32_t unique_tag_cnt_from_input_to_single_output(std::string& input_name, std::string& output_name);
  [[nodiscard]] std::deque<instantiation_graph::vertex_descriptor> least_toggled_path() {
    auto is_excluded_signal = [&](const std::string& name) {
      return std::ranges::any_of(get_excluded_signals(), [&](const std::string& element) {
        return boost::algorithm::contains(name, element);
      });
    };

    auto edge_comparison = [&](const instantiation_graph::edge_descriptor& edge1, const instantiation_graph::edge_descriptor& edge2) {
      return get_signal_values(edge1).m_timed_signal_values.size() < get_signal_values(edge2).m_timed_signal_values.size();
    };

    auto edge_filter = [&](const instantiation_graph::edge_descriptor& edge) { return !is_excluded_signal(m_graph[edge].source_port_ptr->m_name) &&
                                                                                      !is_excluded_signal(m_graph[edge].target_port_ptr->m_name); };

    const auto edge_selector = std::ranges::min_element;

    auto edges = boost::edges(m_graph);
    auto all_edges_filtered = std::vector<instantiation_graph::edge_descriptor>{edges.first, edges.second} | std::views::filter(edge_filter);
    Expects(!all_edges_filtered.empty());
    // get initial edge
    const auto initial_edge = edge_selector(all_edges_filtered, edge_comparison);
    Expects(initial_edge != all_edges_filtered.end());

    const std::string search_direction = "both";
    const std::string result_type = "path";

    return get_path(*initial_edge, edge_comparison, edge_filter, edge_selector, search_direction, result_type);
  }

  [[nodiscard]] std::deque<instantiation_graph::vertex_descriptor> most_toggled_path() {
    auto is_excluded_signal = [&](const std::string& name) {
      return std::ranges::any_of(get_excluded_signals(), [&](const std::string& element) {
        return boost::algorithm::contains(name, element);
      });
    };

    auto edge_comparison = [&](const instantiation_graph::edge_descriptor& edge1, const instantiation_graph::edge_descriptor& edge2) {
      return get_signal_values(edge1).m_timed_signal_values.size() < get_signal_values(edge2).m_timed_signal_values.size();
    };

    auto edge_filter = [&](const instantiation_graph::edge_descriptor& edge) { return !is_excluded_signal(m_graph[edge].source_port_ptr->m_name) &&
                                                                                      !is_excluded_signal(m_graph[edge].target_port_ptr->m_name); };

    const auto edge_selector = std::ranges::max_element;

    auto edges = boost::edges(m_graph);
    auto all_edges_filtered = std::vector<instantiation_graph::edge_descriptor>{edges.first, edges.second} | std::views::filter(edge_filter);
    Expects(!all_edges_filtered.empty());
    // get initial edge
    const auto initial_edge = edge_selector(all_edges_filtered, edge_comparison);
    Expects(initial_edge != all_edges_filtered.end());

    const auto* const search_direction = "both";
    const auto* const result_type = "path";

    return get_path(*initial_edge, edge_comparison, edge_filter, edge_selector, search_direction, result_type);
  }

  [[nodiscard]] std::deque<instantiation_graph::vertex_descriptor> least_tagged_path() {
    auto is_excluded_signal = [&](const std::string& name) {
      return std::ranges::any_of(get_excluded_signals(), [&](const std::string& element) {
        return boost::algorithm::contains(name, element);
      });
    };

    auto count_tags_over_time = [&](const auto& edge) {
      std::size_t sum = 0;
      for (const auto& tag_vector: get_tag_values(edge)) {
        for (const auto& tag_value: tag_vector.m_timed_signal_values) {
          auto val = std::ranges::count_if(tag_value.value.get_value_vector(), [](const auto& tag) { return tag == SignalBit::BIT_1; });
          sum += gsl::narrow<std::size_t>(val);
        }
      }
      return sum;
    };

    auto edge_comparison = [&](const instantiation_graph::edge_descriptor& edge1, const instantiation_graph::edge_descriptor& edge2) {
      return count_tags_over_time(edge1) < count_tags_over_time(edge2);
    };

    auto edge_filter = [&](const instantiation_graph::edge_descriptor& edge) { return !is_excluded_signal(m_graph[edge].source_port_ptr->m_name) &&
                                                                                      !is_excluded_signal(m_graph[edge].target_port_ptr->m_name); };

    const auto edge_selector = std::ranges::min_element;

    auto edges = boost::edges(m_graph);
    auto all_edges_filtered = std::vector<instantiation_graph::edge_descriptor>{edges.first, edges.second} | std::views::filter(edge_filter);
    Expects(!all_edges_filtered.empty());
    // get initial edge
    const auto initial_edge = edge_selector(all_edges_filtered, edge_comparison);
    Expects(initial_edge != all_edges_filtered.end());

    const auto* const search_direction = "both";
    const auto* const result_type = "path";

    return get_path(*initial_edge, edge_comparison, edge_filter, edge_selector, search_direction, result_type);
  }

  [[nodiscard]] std::deque<instantiation_graph::vertex_descriptor> most_tagged_path() {
    auto is_excluded_signal = [&](const std::string& name) {
      return std::ranges::any_of(get_excluded_signals(), [&](const std::string& element) {
        return boost::algorithm::contains(name, element);
      });
    };

    auto count_tags_over_time = [&](const instantiation_graph::edge_descriptor& edge) {
      std::size_t sum = 0;
      for (const auto& tag_vector: get_tag_values(edge)) {
        for (const auto& tag_value: tag_vector.m_timed_signal_values) {
          auto val = std::ranges::count_if(tag_value.value.get_value_vector(), [](const auto& tag) { return tag == SignalBit::BIT_1; });
          sum += gsl::narrow<std::size_t>(val);
        }
      }
      return sum;
    };

    auto edge_comparison = [&](const instantiation_graph::edge_descriptor& edge1, const instantiation_graph::edge_descriptor& edge2) {
      return count_tags_over_time(edge1) < count_tags_over_time(edge2);
    };

    auto edge_filter = [&](const instantiation_graph::edge_descriptor& edge) { return !is_excluded_signal(m_graph[edge].source_port_ptr->m_name) &&
                                                                                      !is_excluded_signal(m_graph[edge].target_port_ptr->m_name); };

    const auto edge_selector = std::ranges::max_element;

    auto edges = boost::edges(m_graph);
    auto edge_vector = std::vector<instantiation_graph::edge_descriptor>{edges.first, edges.second};
    auto all_edges_filtered = edge_vector | std::views::filter(edge_filter);
    Expects(!all_edges_filtered.empty());
    // get initial edge
    const auto initial_edge = edge_selector(all_edges_filtered, edge_comparison);
    Expects(initial_edge != all_edges_filtered.end());

    const auto* const search_direction = "both";
    const auto* const result_type = "path";

    return get_path(*initial_edge, edge_comparison, edge_filter, edge_selector, search_direction, result_type);
  }

  [[nodiscard]] bool signal_contains_x_values(instantiation_graph::edge_descriptor& edge) const {
    const auto signal_values = get_signal_values(edge);
    for (const auto& signal_value: signal_values.m_timed_signal_values) {
      if (signal_value.value.get_type() == SignalValueType::VECTOR) {
        for (const auto& signal_bit: signal_value.value.get_value_vector()) {
          if (signal_bit == SignalBit::BIT_X) {
            return true;
            break;
          }
        }
      } else if (signal_value.value.get_type() == SignalValueType::SCALAR) {
        if (signal_value.value.get_value_bit() == SignalBit::BIT_X) {
          return true;
        }
      }
    }
    return false;
  }

  [[nodiscard]] bool signal_tags_contains_x_values(const instantiation_graph::edge_descriptor& edge) const {
    for (const auto& tag_value: get_tag_values(edge)) {
      for (const auto& tag_val: tag_value.m_timed_signal_values) {
        if (tag_val.value.get_type() == SignalValueType::VECTOR) {
          for (const auto& signal_bit: tag_val.value.get_value_vector()) {
            if (signal_bit == SignalBit::BIT_X) {
              return true;
              break;
            }
          }
        } else if (tag_val.value.get_type() == SignalValueType::SCALAR) {
          if (tag_val.value.get_value_bit() == SignalBit::BIT_X) {
            return true;
          }
        }
      }
    }
    return false;
  }

  [[nodiscard]] std::deque<instantiation_graph::vertex_descriptor> x_tag_path() const {
    auto edge_select_condition = [&](const instantiation_graph::edge_descriptor& edge) {
      return signal_tags_contains_x_values(edge);
    };

    auto edge_filter = [&](const instantiation_graph::edge_descriptor& edge) { return !m_signal_tag_map->get_signal({.name = m_graph[edge].net_ptr->get_name(), .hierarchy = m_graph[edge].hierarchy}).reference.empty(); };

    const auto edge_selector = [&](auto& all_edges_filtered, const auto& select_condition) {
      for (auto it = all_edges_filtered.begin(); it != all_edges_filtered.end(); ++it) {
        if (select_condition(*it)) {
          return it;
        }
      }
      return all_edges_filtered.end();
    };

    auto edges = boost::edges(m_graph);
    auto all_edges_filtered = std::vector<instantiation_graph::edge_descriptor>{edges.first, edges.second} | std::views::filter(edge_filter);
    Expects(!all_edges_filtered.empty());
    // get initial edge
    const auto initial_edge = edge_selector(all_edges_filtered, edge_select_condition);
    if (initial_edge == all_edges_filtered.end()) {
      return {};
    }

    const auto* const search_direction = "both";
    const auto* const result_type = "path";

    return get_path(*initial_edge, edge_select_condition, edge_filter, edge_selector, search_direction, result_type);
  }

  /**
   * @brief This function returns the path that follows a single tag from input to an output
   *
   * to update: with new abstraction class would be to, in this function, do a DFS with is_tag_propagated() the decider which outgoing_edge to go next until an output is reached
  */
  [[nodiscard]] std::deque<instantiation_graph::vertex_descriptor> tag_path_from_input(std::string& input_name, SignalTime tag_vcd_time) {
    //use the initial edge and the tag_vcd_time to find the vcd_file and tag_value for the wanted tag
    //loop through the tags of the initial edge and find the vcd file in which the initial edge was tagged for the specified time
    auto tag = [&](const instantiation_graph::edge_descriptor& edge) {
      auto tag_values = get_tag_values(edge);
      for (const auto& [index, tag_value]: std::views::enumerate(tag_values)) {
        for (const auto& tag_val: tag_value.m_timed_signal_values) {
          if (tag_val.time == tag_vcd_time) {
            return std::make_pair(static_cast<uint32_t>(index), tag_val.value);
          }
        }
      }
      return std::make_pair(static_cast<uint32_t>(0), SignalValue{});
    };

    auto is_excluded_signal = [&](const std::string& name) {
      return std::ranges::any_of(get_excluded_signals(), [&](const std::string& element) {
        return boost::algorithm::contains(name, element);
      });
    };
    auto edge_filter = [&](const instantiation_graph::edge_descriptor& edge) { return !is_excluded_signal(m_graph[edge].source_port_ptr->m_name) &&
                                                                                      !is_excluded_signal(m_graph[edge].target_port_ptr->m_name) && !m_signal_tag_map->is_empty(); };
    // auto edge_filter = [&](const auto& edge) { return static_cast<bool>(!edge.get_property().tags_signal.empty()); };//get all edges with at least one tag

    auto all_edges = boost::edges(m_graph);
    auto all_edges_vector = std::vector<instantiation_graph::edge_descriptor>{all_edges.first, all_edges.second};
    auto all_edges_filtered = all_edges_vector | std::views::filter(edge_filter);
    Expects(!all_edges_filtered.empty());

    auto get_initial_edge = [&](auto& filtered_edges) {
      for (auto edge_it = filtered_edges.begin(); edge_it != filtered_edges.end(); ++edge_it) {
        if (m_graph[*edge_it].source_port_ptr->m_name == input_name) {
          return edge_it;
        }
      }
      return filtered_edges.end();
    };

    // get initial edge
    const auto initial_edge = get_initial_edge(all_edges_filtered);
    if (initial_edge == all_edges_filtered.end()) {
      throw std::runtime_error("initial_edge not found");
    }

    auto input_tag = tag(*initial_edge);
    if (input_tag.second.get_type() == SignalValueType::EMPTY) {
      throw std::runtime_error("initial_edge is empty");
    }

    // select_condition is a lambda that returns true if the edge contains the wanted tag value (wanted tag value is the tag that was injected for the input signal at the specified time)
    auto edge_select_condition = [&](const instantiation_graph::edge_descriptor& edge) {
      const auto& tag_value = get_tag_values(edge);
      const auto& tag_vector = tag_value[input_tag.first];
      return std::ranges::any_of(tag_vector.m_timed_signal_values, [&](const auto& tag_val) {
        if (tag_val.value.get_type() == SignalValueType::VECTOR && input_tag.second.get_type() == SignalValueType::VECTOR) {
          return bitwise_signal_compare(tag_val.value.get_value_vector(), input_tag.second.get_value_vector());
        }
        return false;
      });
    };


    const auto edge_selector = [&](auto& edges_filtered, const auto& select_condition) {
      for (auto it = edges_filtered.begin(); it != edges_filtered.end(); ++it) {
        if (select_condition(*it)) {
          return it;
        }
      }
      return edges_filtered.end();
    };

    const auto* const search_direction = "both";
    const auto* const result_type = "path";

    return get_path(*initial_edge, edge_select_condition, edge_filter, edge_selector, search_direction, result_type);
  }

  [[nodiscard]] std::optional<bool> if_signal_to_signal(SignalIdentifier& input_signal, SignalTime input_time, SignalIdentifier& target_signal, SignalTime target_time) const;
  [[nodiscard]] bool if_signal_to_signals(SignalIdentifier& input_signal, SignalTime input_time, std::vector<SignalIdentifier>& target_signals, SignalTime target_time) const;
};

/*
 * parameters:
 *  - auto initial_edge: starting condition
 *  - auto edge_comparison and auto edge_selector: Predecessor/Successor selection condition
 *  - auto result_type: single path, multiple paths or full subcircuit
 *  - auto search_direction: forward, backward or both
 */
std::deque<ducode::instantiation_graph::vertex_descriptor> DesignInstance::get_path(const instantiation_graph::edge_descriptor& initial_edge, const auto& edge_comparison, const auto& edge_filter, const auto& edge_selector, const std::string& search_direction, const std::string& result_type) const {

  std::deque<instantiation_graph::vertex_descriptor> path;
  std::deque<instantiation_graph::vertex_descriptor> visited;

  auto source_vertex = boost::source(initial_edge, m_graph);
  auto target_vertex = boost::target(initial_edge, m_graph);

  path.emplace_back(source_vertex);
  path.emplace_back(target_vertex);
  visited.emplace_back(source_vertex);
  visited.emplace_back(target_vertex);

  if (search_direction == "backwards" || search_direction == "both") {
    // backward direction
    while (boost::in_degree(source_vertex, m_graph) != 0 || !(m_graph[source_vertex].port_ptr != nullptr ? m_graph[source_vertex].port_ptr->m_direction == Port::Direction::input : false)) {

      auto visited_filter = [&](const instantiation_graph::edge_descriptor& edge) {
        return !contains(visited, boost::target(edge, m_graph));
      };

      auto in_edges = boost::in_edges(source_vertex, m_graph);
      auto in_edges_vector = std::vector<ducode::instantiation_graph::edge_descriptor>{in_edges.first, in_edges.second};
      auto in_edges_filtered = in_edges_vector | std::views::filter(edge_filter) | std::views::filter(visited_filter);
      if (std::distance(in_edges_filtered.begin(), in_edges_filtered.end()) == 0 && result_type == "path") {
        if (path.size() == 1 || path.front() == boost::source(initial_edge, m_graph)) {// path can only backtrack to the initial element of the path
          break;
        }
        path.pop_front();
        source_vertex = path.front();
        continue;/// if in_edges_filtered is empty then break
      }
      auto edge = edge_selector(in_edges_filtered, edge_comparison);
      // do i need this path check now that i have the visited vector????
      //      if (contains(path, edge->get_target()) && result_type == "path") {
      //        path.pop_front();
      //        source_vertex = m_graph.m_vertices[path.front()];
      //        continue;;
      //      }

      auto target = boost::target(*edge, m_graph);
      path.emplace_front(target);
      visited.emplace_front(target);
      source_vertex = target;
    }
  }

  if (search_direction == "forwards" || search_direction == "both") {
    source_vertex = boost::target(initial_edge, m_graph);

    // forward direction
    while (boost::out_degree(source_vertex, m_graph) != 0 || !(m_graph[source_vertex].port_ptr != nullptr ? m_graph[source_vertex].port_ptr->m_direction == Port::Direction::output : false)) {
      auto visited_filter = [&](const instantiation_graph::edge_descriptor& edge) { return !ducode::contains(visited, boost::target(edge, m_graph)); };

      auto out_edges = boost::out_edges(source_vertex, m_graph);
      auto out_edges_vector = std::vector<instantiation_graph::edge_descriptor>{out_edges.first, out_edges.second};
      auto out_edges_filtered = out_edges_vector | std::views::filter(edge_filter) | std::views::filter(visited_filter);
      if (std::distance(out_edges_filtered.begin(), out_edges_filtered.end()) == 0 && result_type == "path") {
        if (path.size() == 1 || path.back() == boost::target(initial_edge, m_graph)) {// path can only backtrack to the initial element of the path
          break;
        }
        path.pop_back();
        source_vertex = path.back();
        continue;
      }
      auto edge = edge_selector(out_edges_filtered, edge_comparison);

      auto target = boost::target(*edge, m_graph);
      path.emplace_back(target);
      visited.emplace_back(target);
      source_vertex = target;
    }
  }

  return path;
}

}// namespace ducode


template<>
struct std::hash<ducode::instantiation_graph::edge_descriptor> {
  std::size_t operator()(const ducode::instantiation_graph::edge_descriptor& edge) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, edge.m_source);
    boost::hash_combine(seed, edge.m_target);
    return seed;
  }
};
