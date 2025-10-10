/* SPDX-License-Identifier: Apache-2.0 */

#include <ducode/bit.hpp>
#include <ducode/cell/mem_v2.hpp>
#include <ducode/instantiation_graph.hpp>
#include <ducode/instantiation_graph_traits.hpp>
#include <ducode/port.hpp>
#include <ducode/types.hpp>
#include <ducode/utility/legalize_identifier.hpp>
#include <ducode/utility/smt.hpp>
#include <ducode/utility/subrange.hpp>

#include <boost/algorithm/string/join.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/detail/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/range/iterator_range_core.hpp>
#include <boost/tuple/tuple.hpp>
#include <fmt/core.h>
#include <fmt/ranges.h>// NOLINT(misc-include-cleaner)
#include <gsl/assert>
#include <nlohmann/json_fwd.hpp>
#include <spdlog/spdlog.h>
#include <z3++.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <limits>
#include <optional>
#include <ranges>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace ducode {

inline const std::unordered_set<std::string> dff_types{"$dff",
                                                       "$adff",
                                                       "$adffe",
                                                       "$adlatch",
                                                       "$aldff",
                                                       "$aldffe",
                                                       "$dffe",
                                                       "$sdff",
                                                       "$sdffe",
                                                       "$dlatch",
                                                       "$dlatchsr",
                                                       "$dffsr",
                                                       "$dffsre"};

inline const std::unordered_set<std::string> dff_types_with_enable{
    "$adffe",
    "$aldffe",
    "$dffe",
    "$sdffe",
    "$dffsre"};

namespace {
//collect matching partial bits and add the corresponding expressions to the port_expr vector
void get_port_expr(const auto& edge, z3::expr_vector& port_expr, std::unordered_set<Bit>& bits, std::vector<Bit>& bits_vector, const bit_container& port_bits, const auto& ev_edge) {
  for (const auto& [source_index, source_bit]: std::views::enumerate(edge.get_property().net_ptr->get_bits())) {
    if (!source_bit.is_constant()) {
      for (const auto& [target_index, target_bit]: std::views::enumerate(port_bits)) {
        if (!target_bit.is_constant()) {
          if (source_bit == target_bit && !bits.contains(source_bit)) {
            auto source_index_string = edge.get_property().net_ptr->get_bits().size() > 1 ? source_index : 0;
            port_expr.push_back(ev_edge.extract(gsl::narrow<uint32_t>(source_index_string), gsl::narrow<uint32_t>(source_index_string)));
            bits.insert(source_bit);
            bits_vector.emplace_back(source_bit);
          }
        }
      }
    }
  }
};
}// namespace

namespace {
z3::expr_vector sort_expr_vector(z3::context& ctx, const z3::expr_vector& predecessors, const BitVector& bit_vector, const std::vector<Bit>& port_bits) {
  z3::expr_vector sorted_expr_vector(ctx);
  for (uint64_t idx = port_bits.size(); idx > 0; idx--) {
    for (const auto& [index, source_bit]: std::views::enumerate(bit_vector)) {
      if (source_bit == port_bits[idx - 1]) {
        sorted_expr_vector.push_back(predecessors[gsl::narrow<uint32_t>(index)]);
      }
    }
  }
  return sorted_expr_vector;
};
}// namespace

[[nodiscard]] TagSourceList DesignInstance::get_information_flows_for_output_from_inputs_at_timestep(const std::vector<SignalIdentifier>& source_ids, SignalIdentifier& target_id, SignalTime timepoint) const {
  return m_signal_tag_map->get_information_flow_source_signals_for_target_signal_at_timestep(source_ids, target_id, timepoint);
};

[[nodiscard]] TagSourceList DesignInstance::get_information_flows_for_output_from_inputs(const std::vector<SignalIdentifier>& source_ids, SignalIdentifier& target_id) const {
  return m_signal_tag_map->get_information_flow_source_signals_for_target_signal(source_ids, target_id);
};

void DesignInstance::set_smt_signal_cell_predecessor_connections(z3::context& ctx, uint32_t design_instance_id, const z3::expr_vector& ev_edges, const std::unordered_map<std::string, uint32_t>& signal_name_to_edge_id, std::unordered_map<std::string, z3::expr>& port_map, const instantiation_graph::vertex_descriptor& vertex, SignalTime unroll_factor) {
  for (const auto& port: m_graph.m_vertices[vertex].m_property.cell_ptr->get_ports()) {
    if (port.m_direction == Port::Direction::input) {
      z3::expr_vector port_expr(ctx);
      std::unordered_set<Bit> bits;
      BitVector bits_vector;
      port_expr.push_back(ctx.bv_val(1, 1));
      bits_vector.emplace_back(1);
      port_expr.push_back(ctx.bv_val(0, 1));
      bits_vector.emplace_back(0);
      port_expr.push_back(ctx.bv_const("x", 1));
      bits_vector.emplace_back(std::numeric_limits<std::size_t>::max());
      port_expr.push_back(ctx.bv_const("z", 1));
      bits_vector.emplace_back(std::numeric_limits<std::size_t>::max() - 1);
      bool matching_edge_found = false;
      if (port.contains_non_constant()) {
        for (const auto& in_edge: m_graph.m_vertices[vertex].m_in_edges) {

          auto time = unroll_factor;
          if (m_graph[vertex].cell_ptr->has_register() && unroll_factor > 0) {
            time = unroll_factor - 1;
          }

          auto signal_name = create_smt_signal_name(design_instance_id, in_edge.get_property().hierarchy, legalize_identifier(in_edge.get_property().net_ptr->get_name()), time);
          if (in_edge.get_property().net_ptr->get_bits_signature() == port.m_bits_signature) {
            if (!signal_name_to_edge_id.contains(signal_name)) {
              throw std::runtime_error("no smt signal with that name found");
            }
            port_map.insert({port.m_name, ev_edges[signal_name_to_edge_id.at(signal_name)]});
            matching_edge_found = true;
            break;// matching bit signature means that we found the correct, matching port
          }
          if (vectors_intersection(in_edge.get_property().net_ptr->get_fast_search_bits(), port.m_fast_search_bits)) {
            //  if bits_signature does not match for the correct port, then call the cell smt function for every matching bit pair
            //  also check if there are constants and add them as bits when necessary
            if (!signal_name_to_edge_id.contains(signal_name)) {
              throw std::runtime_error("no smt signal with that name found");
            }
            get_port_expr(in_edge, port_expr, bits, bits_vector, port.m_bits, ev_edges[signal_name_to_edge_id.at(signal_name)]);
          }
        }
        if (port_expr.size() > 3 && !matching_edge_found) {
          z3::expr_vector preds = sort_expr_vector(ctx, port_expr, bits_vector, port.m_bits);// sort_expr_vector has to fill in any constant 1 or 0
          port_map.insert({port.m_name, z3::concat(preds)});
          //concatenate all bits going to the same port to create a single expression
          // add the port name so the gate can use it to find the correct predecessor expression for each port
        }
      } else if (port.contains_only_constants() && !port.m_bits.empty()) {
        z3::expr_vector preds = sort_expr_vector(ctx, port_expr, bits_vector, port.m_bits);// sort_expr_vector has to fill in any constant 1 or 0
        port_map.insert({port.m_name, z3::concat(preds)});
      }
    }
  }
  Ensures(!port_map.empty());
}

inline void DesignInstance::set_smt_signal_module_port_predecessor_connections(z3::context& ctx, uint32_t design_instance_id, const z3::expr_vector& ev_edges, const std::unordered_map<std::string, uint32_t>& signal_name_to_edge_id, std::unordered_map<std::string, z3::expr>& port_map, const instantiation_graph::vertex_descriptor& vertex, SignalTime unroll_factor) {
  ducode::Port current_port_info;
  // even though cell_ptr->port_list is empty for an output port of the top module the incoming edges are not empty
  // so the logic below has to be used even though the port list is empty
  if (m_graph.m_vertices[vertex].m_property.cell_ptr->get_ports().empty() && m_graph.m_vertices[vertex].m_property.port_ptr->m_direction == Port::Direction::input) {
    auto signal_name = create_smt_signal_name(design_instance_id, m_graph.m_vertices[vertex].m_property.hierarchy, legalize_identifier(m_graph.m_vertices[vertex].m_property.port_ptr->m_name), unroll_factor);
    assert(signal_name_to_edge_id.contains(signal_name));
    port_map.insert({m_graph.m_vertices[vertex].m_property.port_ptr->m_name, ev_edges[signal_name_to_edge_id.at(signal_name)]});
    return;
  }
  if (m_graph.m_vertices[vertex].m_property.port_ptr->m_direction == ducode::Port::Direction::output) {
    current_port_info = *m_graph.m_vertices[vertex].m_property.port_ptr;
  } else {
    for (const auto& port: m_graph.m_vertices[vertex].m_property.cell_ptr->get_ports()) {
      if (m_graph.m_vertices[vertex].m_property.port_ptr != nullptr) {
        if (m_graph.m_vertices[vertex].m_property.port_ptr->m_name == port.m_name) {
          current_port_info = port;
          break;
          ;
        }
      } else {
        throw std::runtime_error("Port pointer of a cell_module is null");
      }
    }
  }
  z3::expr_vector port_expr(ctx);
  std::unordered_set<Bit> bits;
  BitVector bits_vector;
  port_expr.push_back(ctx.bv_val(1, 1));
  bits_vector.emplace_back(1);
  port_expr.push_back(ctx.bv_val(0, 1));
  bits_vector.emplace_back(0);
  port_expr.push_back(ctx.bv_const("x", 1));
  bits_vector.emplace_back(std::numeric_limits<std::size_t>::max());
  bool matching_edge_found = false;
  if (current_port_info.contains_non_constant()) {
    for (const auto& in_edge: m_graph.m_vertices[vertex].m_in_edges) {
      auto signal_name = create_smt_signal_name(design_instance_id, in_edge.get_property().hierarchy, legalize_identifier(in_edge.get_property().net_ptr->get_name()), unroll_factor);
      if (in_edge.get_property().net_ptr->get_bits_signature() == current_port_info.m_bits_signature) {// not enough to check; bits can partly match
        if (!signal_name_to_edge_id.contains(signal_name)) {
          throw std::runtime_error("no smt signal with that name found");
        }
        port_map.insert({current_port_info.m_name, ev_edges[signal_name_to_edge_id.at(signal_name)]});
        matching_edge_found = true;
        break;// matching bit signature means that we found the correct, matching port
      }
      if (vectors_intersection(in_edge.get_property().net_ptr->get_fast_search_bits(), current_port_info.m_fast_search_bits)) {
        //  if bits_signature does not match for the correct port, then call the cell smt function for every matching bit pair
        if (!signal_name_to_edge_id.contains(signal_name)) {
          throw std::runtime_error("no smt signal with that name found");
        }
        get_port_expr(in_edge, port_expr, bits, bits_vector, current_port_info.m_bits, ev_edges[signal_name_to_edge_id.at(signal_name)]);
      }
    }
    if (port_expr.size() > 3 && !matching_edge_found) {
      z3::expr_vector preds = sort_expr_vector(ctx, port_expr, bits_vector, current_port_info.m_bits);
      port_map.insert({current_port_info.m_name, z3::concat(preds)});
    }
  } else if (current_port_info.contains_only_constants()) {
    z3::expr_vector preds = sort_expr_vector(ctx, port_expr, bits_vector, current_port_info.m_bits);// sort_expr_vector has to fill in any constant 1 or 0
    port_map.insert({current_port_info.m_name, z3::concat(preds)});
  }
  Ensures(!port_map.empty());
};

void DesignInstance::set_smt_signal_module_port_successor_connections(z3::context& ctx, uint32_t design_instance_id, const z3::expr_vector& ev_edges, const std::unordered_map<std::string, uint32_t>& signal_name_to_edge_id, std::unordered_map<std::string, z3::expr>& port_expr_map, const instantiation_graph::vertex_descriptor& vertex, SignalTime unroll_factor) {
  bit_container module_port_bits;
  fast_bit_container module_port_fast_search_bits;
  size_t bits_signature = 0;
  if (m_graph.m_vertices[vertex].m_property.cell_ptr->get_ports().empty() && m_graph.m_vertices[vertex].m_property.port_ptr->m_direction == Port::Direction::output) {
    auto signal_name = create_smt_signal_name(design_instance_id, m_graph.m_vertices[vertex].m_property.hierarchy, legalize_identifier(m_graph.m_vertices[vertex].m_property.port_ptr->m_name), unroll_factor);
    assert(signal_name_to_edge_id.contains(signal_name));
    port_expr_map.insert({fmt::format("{}_successor", m_graph.m_vertices[vertex].m_property.port_ptr->m_name), ev_edges[signal_name_to_edge_id.at(signal_name)]});
  } else if (m_graph.m_vertices[vertex].m_property.port_ptr->m_direction == Port::Direction::input || (m_graph.m_vertices[vertex].m_property.port_ptr->m_direction == Port::Direction::inout && m_graph.m_vertices[vertex].m_in_edges.empty())) {
    module_port_bits = m_graph.m_vertices[vertex].m_property.port_ptr->m_bits;
    bits_signature = m_graph.m_vertices[vertex].m_property.port_ptr->m_bits_signature;
    module_port_fast_search_bits = m_graph.m_vertices[vertex].m_property.port_ptr->m_fast_search_bits;
  } else {
    assert(!m_graph.m_vertices[vertex].m_property.cell_ptr->get_ports().empty());
    for (const auto& port: m_graph.m_vertices[vertex].m_property.cell_ptr->get_ports()) {
      if (m_graph.m_vertices[vertex].m_property.port_ptr != nullptr) {
        if (m_graph.m_vertices[vertex].m_property.port_ptr->m_name == port.m_name) {
          module_port_bits = port.m_bits;
          bits_signature = port.m_bits_signature;
          module_port_fast_search_bits = port.m_fast_search_bits;
          break;
        }
      } else {
        throw std::runtime_error("Port pointer of a cell_module is null");
      }
    }
  }
  z3::expr_vector port_expr(ctx);
  std::unordered_set<Bit> bits;
  BitVector bits_vector;
  if (!m_graph.m_vertices[vertex].m_out_edges.empty()) {
    for (const auto& out_edge: m_graph.m_vertices[vertex].m_out_edges) {
      auto signal_name = create_smt_signal_name(design_instance_id, out_edge.get_property().hierarchy, legalize_identifier(out_edge.get_property().net_ptr->get_name()), unroll_factor);
      if (out_edge.get_property().net_ptr->get_bits_signature() == bits_signature) {// not enough to check; bits can partly match
        if (!signal_name_to_edge_id.contains(signal_name)) {
          throw std::runtime_error("no smt signal with that name found");
        }
        port_expr_map.insert({fmt::format("{}_successor", m_graph.m_vertices[vertex].m_property.port_ptr->m_name), ev_edges[signal_name_to_edge_id.at(signal_name)]});
        break;// matching bit signature means that we found the correct, matching port
      }
      if (vectors_intersection(out_edge.get_property().net_ptr->get_fast_search_bits(), module_port_fast_search_bits)) {
        //  if bits_signature does not match for the correct port, then call the cell smt function for every matching bit pair
        if (!signal_name_to_edge_id.contains(signal_name)) {
          throw std::runtime_error("no smt signal with that name found");
        }
        get_port_expr(out_edge, port_expr, bits, bits_vector, module_port_bits, ev_edges[signal_name_to_edge_id.at(signal_name)]);
      }
    }
    if (!port_expr.empty()) {// called even when a matching edge is found but due to using port_expr_map.insert the found signal is not overwritten
      z3::expr_vector succs = sort_expr_vector(ctx, port_expr, bits_vector, module_port_bits);
      port_expr_map.insert({fmt::format("{}_successor", m_graph.m_vertices[vertex].m_property.port_ptr->m_name), z3::concat(succs)});//concatenate all bits going to the same port to create a single expression
    }
  }
};

void DesignInstance::set_smt_signal_cell_successor_connections(z3::context& ctx, uint32_t design_instance_id, const z3::expr_vector& ev_edges, const std::unordered_map<std::string, uint32_t>& signal_name_to_edge_id, std::unordered_map<std::string, z3::expr>& port_expr_map, const instantiation_graph::vertex_descriptor& vertex, SignalTime unroll_factor, bool dff_enable) {
  for (const auto& port: m_graph.m_vertices[vertex].m_property.cell_ptr->get_ports()) {
    if (port.m_direction == Port::Direction::output) {
      z3::expr_vector port_expr(ctx);
      std::unordered_set<Bit> bits;
      BitVector bits_vector;
      Ensures(!m_graph.m_vertices[vertex].m_out_edges.empty());
      for (const auto& out_edge: m_graph.m_vertices[vertex].m_out_edges) {
        auto signal_name = create_smt_signal_name(design_instance_id, out_edge.get_property().hierarchy, legalize_identifier(out_edge.get_property().net_ptr->get_name()), unroll_factor);
        if (out_edge.get_property().net_ptr->get_bits_signature() == port.m_bits_signature) {// not enough to check; bits can partly match
          if (!signal_name_to_edge_id.contains(signal_name)) {
            throw std::runtime_error(fmt::format("no smt signal with that name found: {}", signal_name));
          }
          z3::expr_vector predecessor(ctx);
          dff_enable ? port_expr_map.insert({fmt::format("{}_prev", port.m_name), ev_edges[signal_name_to_edge_id.at(signal_name)]}) : port_expr_map.insert({port.m_name, ev_edges[signal_name_to_edge_id.at(signal_name)]});
          break;// matching bit signature means that we found the correct, matching port
        }
        if (out_edge.get_property().net_ptr->contains_non_constant() && port.contains_non_constant()) {
          if (vectors_intersection(out_edge.get_property().net_ptr->get_fast_search_bits(), port.m_fast_search_bits)) {
            //  if bits_signature does not match for the correct port, then call the cell smt function for every matching bit pair
            if (!signal_name_to_edge_id.contains(signal_name)) {
              throw std::runtime_error(fmt::format("no smt signal with that name found: {}", signal_name));
            }
            get_port_expr(out_edge, port_expr, bits, bits_vector, port.m_bits, ev_edges[signal_name_to_edge_id.at(signal_name)]);
          }
        }
      }
      if (!port_expr.empty()) {
        z3::expr_vector succs = sort_expr_vector(ctx, port_expr, bits_vector, port.m_bits);
        dff_enable ? port_expr_map.insert({fmt::format("{}_prev", port.m_name), z3::concat(succs)}) : port_expr_map.insert({port.m_name, z3::concat(succs)});
      }
    }
  }
};

void DesignInstance::export_smt2(z3::context& ctx, z3::solver& solver, z3::expr_vector& ev_edges, std::unordered_map<std::string, uint32_t>& signal_name_to_edge_id, uint32_t design_instance_id, SignalTime unroll_factor) {
  auto vectors_intersection = [](const fast_bit_container& bits1, const fast_bit_container& bits2) -> bool {
    if (bits1.empty() || bits2.empty()) {
      throw std::runtime_error("Empty bits!");
    }

    return std::ranges::any_of(bits1, [&](const Bit& bit) { return bits2.contains(bit); });
  };

  nlohmann::json params;
  params["ift"] = false;
  params["identifier_context"] = "";

  std::unordered_map<std::string, uint32_t> input_name_to_signal_id;
  std::unordered_map<std::string, uint32_t> output_name_to_signal_id;

  // create SMT signals/variables for each edge in the graph // edges with the same name are considered to be the same signal/variable
  for (const auto& edge: boost::make_iterator_range(boost::edges(m_graph))) {
    bool contains = false;
    std::string edge_name = create_smt_signal_name(design_instance_id, m_graph[edge].hierarchy, legalize_identifier(m_graph[edge].net_ptr->get_name()), unroll_factor);
    if (!signal_name_to_edge_id.contains(edge_name)) {
      ev_edges.push_back(ctx.bv_const(edge_name.c_str(), static_cast<uint32_t>(m_graph[edge].net_ptr->get_bits().size())));
      signal_name_to_edge_id[edge_name] = gsl::narrow<uint32_t>(ev_edges.size() - 1);
      // adding signals for top module input ports
      if (m_graph.m_vertices[edge.m_source].m_in_edges.empty()) {
        input_name_to_signal_id[fmt::format("D{}_{}", design_instance_id, legalize_identifier(m_graph[edge].net_ptr->get_name()))] = gsl::narrow<uint32_t>(ev_edges.size() - 1);
      } else if (m_graph.m_vertices[edge.m_target].m_out_edges.empty() && m_graph.m_vertices[edge.m_target].m_property.type == "module_port") {
        std::string output_name = create_smt_signal_name(design_instance_id, m_graph.m_vertices[edge.m_target].m_property.hierarchy, legalize_identifier(m_graph.m_vertices[edge.m_target].m_property.port_ptr->m_name), unroll_factor);
        if (!signal_name_to_edge_id.contains(output_name)) {
          ev_edges.push_back(ctx.bv_const(output_name.c_str(), static_cast<uint32_t>(m_graph.m_vertices[edge.m_target].m_property.port_ptr->m_bits.size())));
          signal_name_to_edge_id[output_name] = gsl::narrow<uint32_t>(ev_edges.size() - 1);
        }
        output_name_to_signal_id[fmt::format("D{}_{}", design_instance_id, legalize_identifier(m_graph[edge].net_ptr->get_name()))] = gsl::narrow<uint32_t>(ev_edges.size() - 1);
      }
    }
  }

  for (const auto& vertex: boost::make_iterator_range(boost::vertices(m_graph))) {
    // for all module port cells
    std::unordered_map<std::string, z3::expr> port_map;
    if (m_graph[vertex].cell_ptr->has_register() && unroll_factor == 0) {
      continue;
    }
    if (m_graph[vertex].type == "$mem_v2") {
      // create a memory register for each timestep
      const auto* mem_cell_ptr = dynamic_cast<const ducode::MemV2*>(m_graph[vertex].cell_ptr);
      if (mem_cell_ptr == nullptr) {
        throw std::runtime_error("Memory cell pointer is null");
      }
      if (mem_cell_ptr->rd_ports > 1) {
        throw std::runtime_error("Memory cell has more than one read port");
      }
      if (mem_cell_ptr->wr_ports > 1) {
        throw std::runtime_error("Memory cell has more than one write port");
      }
      z3::sort bv_addr = ctx.bv_sort(static_cast<uint32_t>(std::ceil(std::log2(mem_cell_ptr->size))));// number of address bits; size of the memory
      z3::sort bv_size = ctx.bv_sort(static_cast<uint32_t>(mem_cell_ptr->width));                     // number of data bits; width of one memory register
      z3::sort memory_sort = ctx.array_sort(bv_addr, bv_size);                                        // memory is an array of size bits with address bits

      std::string mem_reg_name = create_smt_signal_name(design_instance_id, m_graph.m_vertices[vertex].m_property.hierarchy, legalize_identifier(m_graph[vertex].cell_ptr->get_name()), unroll_factor);
      ev_edges.push_back(ctx.constant(mem_reg_name.c_str(), memory_sort));
      signal_name_to_edge_id[mem_reg_name] = gsl::narrow<uint32_t>(ev_edges.size() - 1);
      port_map.insert({mem_reg_name, ev_edges[gsl::narrow<uint32_t>(ev_edges.size() - 1)]});
      params["unroll_factor"] = unroll_factor;
      params["mem_reg_name"] = mem_reg_name;
      if (unroll_factor > 0) {
        std::string mem_reg_name_previous = create_smt_signal_name(design_instance_id, m_graph.m_vertices[vertex].m_property.hierarchy, legalize_identifier(m_graph[vertex].cell_ptr->get_name()), unroll_factor - 1);
        port_map.insert({mem_reg_name_previous, ev_edges[signal_name_to_edge_id[mem_reg_name_previous]]});
        params["mem_reg_name_previous"] = mem_reg_name_previous;
      } else {
        std::string mem_reg_name_initial = create_smt_signal_name(design_instance_id, m_graph.m_vertices[vertex].m_property.hierarchy, legalize_identifier(m_graph[vertex].cell_ptr->get_name()), static_cast<uint32_t>(-1));
        ev_edges.push_back(ctx.constant(mem_reg_name_initial.c_str(), memory_sort));
        //function to initialize the memory
        // this is only used for the first timestep
        // mem_v2 cell has a string with the initial values
        // this string has to be parsed and the values have to be added to the memory

        std::vector<z3::expr> init_values;
        std::string init_value(mem_cell_ptr->init);

        // directly convert the string into expressions for z3 instead of using integer values in between because the initstring can contain 'x' or 'z'
        for (size_t i = 0; i < init_value.size(); i += mem_cell_ptr->width) {
          std::string init_value_part = init_value.substr(i, mem_cell_ptr->width);
          if (init_value_part.find('x') != std::string::npos) {
            init_values.push_back(ctx.bv_const("x", gsl::narrow<uint32_t>(mem_cell_ptr->width)));
          } else if (init_value_part.find('z') != std::string::npos) {
            init_values.push_back(ctx.bv_const("z", gsl::narrow<uint32_t>(mem_cell_ptr->width)));
          } else {
            init_values.push_back(ctx.bv_val(gsl::narrow<uint32_t>(std::stoull(init_value_part, nullptr, 2)), gsl::narrow<uint32_t>(mem_cell_ptr->width)));
          }
          // init_values.push_back(ctx.bv_val(init_values[i], mem_cell_ptr->width));// Create bit-vector value
        }
        for (uint64_t addr = 0; addr < init_values.size(); ++addr) {
          solver.add(ev_edges.back() == z3::store(ev_edges.back(), ctx.bv_val(addr, static_cast<uint32_t>(std::ceil(std::log2(mem_cell_ptr->size)))), init_values[addr]));
        }

        signal_name_to_edge_id[mem_reg_name_initial] = gsl::narrow<uint32_t>(ev_edges.size() - 1);
        port_map.insert({mem_reg_name_initial, ev_edges[gsl::narrow<uint32_t>(ev_edges.size() - 1)]});
        params["mem_reg_name_previous"] = mem_reg_name_initial;
      }
    }
    if (m_graph[vertex].type != "module_port" && m_graph.m_vertices[vertex].m_out_edges.empty()) {
      continue;
    }
    if (m_graph[vertex].type == "module_port") {
      if (m_graph.m_vertices[vertex].m_out_edges.empty() && m_graph.m_vertices[vertex].m_property.port_ptr->m_direction == Port::Direction::input) {
        continue;// a module port with not out edges is ignored because it does not effect any other signal
      }
      if (m_graph.m_vertices[vertex].m_in_edges.empty() && m_graph.m_vertices[vertex].m_property.port_ptr->m_direction == Port::Direction::output) {
        continue;
      }
      if (m_graph.m_vertices[vertex].m_property.port_ptr->m_direction == Port::Direction::inout) {
        throw std::runtime_error("Inout ports are not supported");
      }
      if (m_graph.m_vertices[vertex].m_out_edges.empty() && m_graph.m_vertices[vertex].m_property.type == "module_port") {
        std::string output_name = create_smt_signal_name(design_instance_id, m_graph.m_vertices[vertex].m_property.hierarchy, legalize_identifier(m_graph.m_vertices[vertex].m_property.port_ptr->m_name), unroll_factor);
        if (!signal_name_to_edge_id.contains(output_name)) {
          ev_edges.push_back(ctx.bv_const(output_name.c_str(), static_cast<uint32_t>(m_graph.m_vertices[vertex].m_property.port_ptr->m_bits.size())));
          signal_name_to_edge_id[output_name] = gsl::narrow<uint32_t>(ev_edges.size() - 1);
        }
      }
      if (m_graph.m_vertices[vertex].m_in_edges.empty() && m_graph.m_vertices[vertex].m_property.type == "module_port") {
        std::string input_name = create_smt_signal_name(design_instance_id, m_graph.m_vertices[vertex].m_property.hierarchy, legalize_identifier(m_graph.m_vertices[vertex].m_property.port_ptr->m_name), unroll_factor);
        if (!signal_name_to_edge_id.contains(input_name)) {
          ev_edges.push_back(ctx.bv_const(input_name.c_str(), static_cast<uint32_t>(m_graph.m_vertices[vertex].m_property.port_ptr->m_bits.size())));
          signal_name_to_edge_id[input_name] = gsl::narrow<uint32_t>(ev_edges.size() - 1);
        }
      }
      set_smt_signal_module_port_successor_connections(ctx, design_instance_id, ev_edges, signal_name_to_edge_id, port_map, vertex, unroll_factor);
      set_smt_signal_module_port_predecessor_connections(ctx, design_instance_id, ev_edges, signal_name_to_edge_id, port_map, vertex, unroll_factor);
      params["port"] = m_graph[vertex].port_ptr->m_name;
    } else {
      set_smt_signal_cell_successor_connections(ctx, design_instance_id, ev_edges, signal_name_to_edge_id, port_map, vertex, unroll_factor);
      if (dff_types_with_enable.contains(m_graph.m_vertices[vertex].m_property.cell_ptr->get_type()) && unroll_factor >= 1) {
        set_smt_signal_cell_successor_connections(ctx, design_instance_id, ev_edges, signal_name_to_edge_id, port_map, vertex, unroll_factor - 1, true);
      }
      set_smt_signal_cell_predecessor_connections(ctx, design_instance_id, ev_edges, signal_name_to_edge_id, port_map, vertex, unroll_factor);
    }
    write_smt2(solver, "and_test.smt2");

    m_graph[vertex].cell_ptr->export_smt2(ctx, solver, port_map, params);
  }

  spdlog::info("SMT instance {} created; timestep {}", design_instance_id, unroll_factor);
}

void DesignInstance::write_verilog(const std::string& filename) {
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
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;

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
        if (m_graph[*vi].port_ptr->m_direction == Port::Direction::input) {
          result << fmt::format("assign {} = {};\n", name, m_graph[*vi].port_ptr->m_name);
        } else {
          result << fmt::format("assign {} = {};\n", m_graph[*vi].port_ptr->m_name, name);
        }
      }
      // assign eventual constant bits
      if (m_graph[*vi].port_ptr->contains_constant()) {
        for (const auto& [index, bit]: std::views::enumerate(m_graph[*vi].port_ptr->m_bits)) {
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
          for (const auto& [index, value]: std::views::enumerate(port.m_bits)) {
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
    if (m_graph[*ei].source_port_ptr->m_direction == Port::Direction::output && m_graph[boost::source(*ei, m_graph)].type == "module_port") {
      const auto& port_it = std::ranges::find_if(m_graph[boost::source(*ei, m_graph)].cell_ptr->get_ports(),
                                                 [&](const auto& port) {
                                                   return port.m_name == m_graph[*ei].source_port_ptr->m_name;
                                                 });
      Expects(port_it != m_graph[boost::source(*ei, m_graph)].cell_ptr->get_ports().end());
      source_bits = &(port_it->m_bits);
    }

    const auto* target_bits = &(m_graph[*ei].target_port_ptr->m_bits);
    if (m_graph[*ei].target_port_ptr->m_direction == Port::Direction::input && m_graph[boost::target(*ei, m_graph)].type == "module_port") {
      const auto& port_it = std::ranges::find_if(m_graph[boost::target(*ei, m_graph)].cell_ptr->get_ports(),
                                                 [&](const auto& port) {
                                                   return port.m_name == m_graph[*ei].target_port_ptr->m_name;
                                                 });
      target_bits = &(port_it->m_bits);
    }

    if (*target_bits == *source_bits) {
      result << fmt::format("assign {} = {};\n", target_wire, source_wire);
    } else {
      for (const auto& [source_index, source_bit]: std::views::enumerate(*source_bits)) {
        if (!source_bit.is_constant()) {
          for (const auto& [target_index, target_bit]: std::views::enumerate(*target_bits)) {
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

namespace {
/// count the number of appearances of 'elements' in all paths
uint32_t count_element_occurrences(std::vector<std::deque<ducode::instantiation_graph::vertex_descriptor>> paths, std::deque<ducode::instantiation_graph::vertex_descriptor> elements) {
  uint32_t result = 0;
  bool contains = true;
  for (auto& path: paths) {
    if (std::ranges::find(path, elements.at(0)) != path.end()) {
      auto it_path = std::ranges::find(path, elements.at(0));
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
}// namespace

std::vector<std::pair<std::deque<ducode::instantiation_graph::vertex_descriptor>, uint32_t>> DesignInstance::count_common_path_elements(std::vector<std::deque<ducode::instantiation_graph::vertex_descriptor>>& paths) {
  std::deque<instantiation_graph::vertex_descriptor> elements;
  std::vector<std::pair<std::deque<instantiation_graph::vertex_descriptor>, uint32_t>> result;
  for (const auto& path: paths) {
    for (const auto& [index, element]: std::views::enumerate(path)) {
      elements.clear();
      auto uindex = gsl::narrow<uint64_t>(index);
      for (auto i = uindex; i < path.size(); i++) {
        elements.emplace_back(path.at(i));
        if (!std::ranges::any_of(result, [&](const auto& ele) { return ele.first == elements; })) {
          result.emplace_back(elements, count_element_occurrences(paths, elements));
        }
      }
    }
  }
  return result;
}

std::vector<std::pair<std::string, uint32_t>> DesignInstance::count_output_tags() const {
  std::vector<std::pair<std::string, uint32_t>> result;
  auto top_module_outputs = m_design.get_module(m_design.get_top_module_name()).get_output_names();
  auto vertex_filter = [&](const instantiation_graph::vertex_descriptor& vertex) {
    if (m_graph[vertex].port_ptr != nullptr) {
      return ((m_graph[vertex].port_ptr->m_direction == Port::Direction::output) && (std::ranges::find(top_module_outputs, m_graph[vertex].port_ptr->m_name) != top_module_outputs.end()) && (m_graph[vertex].hierarchy.size() == 1));
    }
    return false;
  };
  uint32_t count = 0;

  auto vertices_iterators = boost::vertices(m_graph);
  auto vertex_vector = std::vector<instantiation_graph::vertex_descriptor>{vertices_iterators.first, vertices_iterators.second};
  auto all_output_vertices = vertex_vector | std::views::filter(vertex_filter);
  assert(!all_output_vertices.empty());

  for (const auto& vertex: all_output_vertices) {
    count = 0;
    if (boost::in_degree(vertex, m_graph) == 0) {
      continue;
    }

    auto first_in_edge = *boost::in_edges(vertex, m_graph).first;
    std::vector<SignalValues> tag_vector = get_tag_values(first_in_edge);

    for (auto& tag_values: tag_vector) {
      for (const auto& tags: tag_values.m_timed_signal_values) {
        for (const auto& bit: tags.value.get_value_vector()) {
          if (bit == SignalBit::BIT_1) {
            count++;
          }
        }
      }
    }
    result.emplace_back(m_graph[vertex].name, count);
  }
  return result;
}

bool DesignInstance::bitwise_signal_compare(SignalBitVector vec1, SignalBitVector vec2) {
  if (vec1.size() != vec2.size()) {
    return false;
  }

  SignalBitVector result_vec;
  for (auto i = 0ul; i < vec1.size(); i++) {
    if (vec1[i] == vec2[i] && vec1[i] == SignalBit::BIT_1 && vec2[i] == SignalBit::BIT_1) {
      result_vec.emplace_back(SignalBit::BIT_1);
    } else {
      result_vec.emplace_back(SignalBit::BIT_0);
    }
  }
  return std::ranges::any_of(result_vec, [](auto bit) { return bit == SignalBit::BIT_1; });
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

void DesignInstance::populate_module(instantiation_graph::vertex_descriptor module_vertex, std::vector<std::string> hierarchy, const Cell& cell_module) {
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
    InstantiationGraphVertexInfo module_vertex_prop;
    module_vertex_prop.name = cell.get_name();
    module_vertex_prop.parent_module = module_it->get_name();
    module_vertex_prop.hierarchy = hierarchy;
    module_vertex_prop.type = cell.get_type();
    module_vertex_prop.cell_ptr = &cell;
    new_cell_vertices.emplace_back(boost::add_vertex(module_vertex_prop, m_graph));
  }

  std::unordered_map<Bit, std::unordered_set<std::pair<instantiation_graph::vertex_descriptor, const Port*>, PairHash>> bit_to_target_vertices;

  const auto insert_into_mapset = [&](Bit bit, instantiation_graph::vertex_descriptor vertex, const Port* port) {
    const auto it = bit_to_target_vertices.find(bit);
    if (it != bit_to_target_vertices.end()) {
      it->second.emplace(vertex, port);
    } else {
      bit_to_target_vertices.emplace(bit, std::unordered_set<std::pair<instantiation_graph::vertex_descriptor, const Port*>, PairHash>{{vertex, port}});
    }
  };

  for (const auto& new_cell_vertex: new_cell_vertices) {
    if (m_graph[new_cell_vertex].type == "module_port") {
      if (m_graph[new_cell_vertex].port_ptr->m_direction == Port::Direction::output) {
        for (const auto& bit: m_graph[new_cell_vertex].port_ptr->m_bits) {
          if (!bit.is_constant()) {
            insert_into_mapset(bit, new_cell_vertex, m_graph[new_cell_vertex].port_ptr);
          }
        }
      }
    } else {
      for (const auto& port: m_graph[new_cell_vertex].cell_ptr->get_ports() | std::views::filter([](const Port& port) { return port.m_direction == Port::Direction::input; })) {
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

    const auto connect_port = [&](const Port& port) {
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
      if (m_graph[new_cell_vertex].port_ptr->m_direction == Port::Direction::input) {
        connect_port(*m_graph[new_cell_vertex].port_ptr);
      }
    } else {
      for (const auto& port: m_graph[new_cell_vertex].cell_ptr->get_ports() | std::views::filter([](const Port& port) { return port.m_direction == Port::Direction::output; })) {
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

  auto vectors_intersection = [](const fast_bit_container& bits1, const fast_bit_container& bits2) -> bool {
    if (bits1.empty() || bits2.empty()) {
      throw std::runtime_error("Empty bits!");
    }

    return std::ranges::any_of(bits1, [&](const Bit& bit) { return bits2.contains(bit); });
  };

  // check if there are multiple possible nets for the given bits
  // if multiple nets are found they should have the same bit signature -> net with aliases
  for (const auto& edge: new_edges) {
    const Net* tmp_net = nullptr;
    if (m_graph[edge].net_ptr == nullptr) {
      for (const auto& net_signature: signature_to_net) {
        if (vectors_intersection(m_graph[edge].source_port_ptr->m_fast_search_bits, net_signature.second->get_fast_search_bits()) && vectors_intersection(m_graph[edge].target_port_ptr->m_fast_search_bits, net_signature.second->get_fast_search_bits())) {
          tmp_net = net_signature.second;
          for (const auto& net_signature_tmp: signature_to_net) {
            if ((vectors_intersection(m_graph[edge].source_port_ptr->m_fast_search_bits, net_signature.second->get_fast_search_bits()) || vectors_intersection(m_graph[edge].target_port_ptr->m_fast_search_bits, net_signature.second->get_fast_search_bits())) && net_signature.second->get_name() != tmp_net->get_name() && net_signature.second->get_bits_signature() != tmp_net->get_bits_signature()) {
              throw std::runtime_error("multiple nets with the same bit(s) that are not aliases!!");
            }
          }

          m_graph[edge].net_ptr = net_signature.second;
          break;
        }
      }
      // search for nets with partial bit matches
      // there should only be a single net with matching bits for every edge
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

std::vector<ducode::instantiation_graph::vertex_descriptor> DesignInstance::create_module_ports_vertices(instantiation_graph::vertex_descriptor module_vertex, std::vector<std::string> hierarchy, const Cell& cell_module) {
  Expects(cell_module.is_module());
  Expects(cell_module.get_name() == m_graph[module_vertex].name);

  hierarchy.emplace_back(cell_module.get_name());

  auto module_it = std::ranges::find_if(m_design.get_modules(), [&](const Module& mod) { return cell_module.get_type() == mod.get_name(); });

  if (module_it == m_design.get_modules().end()) {
    throw std::runtime_error("module not found");
  }

  std::vector<instantiation_graph::vertex_descriptor> new_cell_vertices;

  for (const Port& port: module_it->get_ports()) {
    InstantiationGraphVertexInfo module_vertex_prop;
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

ducode::instantiation_graph::edge_descriptor DesignInstance::add_edge(const instantiation_graph::vertex_descriptor& source_vertex, const instantiation_graph::vertex_descriptor& target_vertex, const Port* source_port_ptr, const Port* target_port_ptr) {
  auto edge = boost::add_edge(source_vertex, target_vertex, m_graph).first;

  const std::string name = fmt::format("{} -> {}", source_port_ptr->m_bits, target_port_ptr->m_bits);

  spdlog::debug("Adding edge from {} to {}", source_port_ptr->m_identifier, target_port_ptr->m_identifier);

  m_graph[edge].name = name;
  m_graph[edge].source_port_ptr = source_port_ptr;
  m_graph[edge].target_port_ptr = target_port_ptr;

  return edge;
}

void DesignInstance::propagate_net_pointers_impl(const instantiation_graph::edge_descriptor& current_edge, const Net* previous_edge_net, std::unordered_set<instantiation_graph::edge_descriptor>& visited) {
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

void DesignInstance::propagate_net_pointers(const Module& top_module) {
  std::unordered_set<instantiation_graph::edge_descriptor> visited;

  for (const auto& vertex: subrange(boost::vertices(m_graph))) {
    for (const auto& port: top_module.get_ports()) {
      if (m_graph[vertex].type == "module_port" && m_graph[vertex].name == fmt::format("{}_{}", top_module.get_name(), port.m_name)) {
        spdlog::info("Starting propagation from {}", port.m_name);

        const auto net_it = std::ranges::find_if(top_module.get_nets(), [&](const Net& net) { return net.get_name() == port.m_name; });
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

[[nodiscard]] std::optional<bool> DesignInstance::if_signal_to_signal(SignalIdentifier& input_signal, SignalTime input_time, SignalIdentifier& target_signal, SignalTime target_time) const {

  // if the input signal is the same as the target signal, we can return true
  if (input_signal == target_signal) {
    return true;
  }
  if (m_signal_tag_map == nullptr) {
    throw std::runtime_error("Signal tag map is not initialized");
  }
  return m_signal_tag_map->is_tag_propagated(input_signal, input_time, target_signal, target_time);
}

// counts the number of tags on a single given edge - used here for specific output edges
[[nodiscard]] uint32_t DesignInstance::tag_cnt_single_edge(std::string& edge_name) const {
  uint32_t tag_cnt = 0;
  std::set<uint32_t> tags;
  for (auto [edge_it, edge_end] = boost::edges(m_graph); edge_it != edge_end; ++edge_it) {
    if (m_graph[*edge_it].net_ptr != nullptr && (m_graph[*edge_it].net_ptr->get_name() == edge_name)) {
      tag_cnt = m_signal_tag_map->get_tag_count_signal(fmt::format("{}_t", m_graph[*edge_it].net_ptr->get_name()), m_graph[*edge_it].hierarchy);
      break;
    }
  }
  return tag_cnt;
}

[[nodiscard]] uint32_t DesignInstance::tag_cnt_multiple_edges(std::vector<std::string>& edge_names) const {
  uint32_t tag_cnt = 0;
  std::set<uint32_t> tags;
  for (auto& edge_name: edge_names) {
    for (auto [edge_it, edge_end] = boost::edges(m_graph); edge_it != edge_end; ++edge_it) {
      if (m_graph[*edge_it].net_ptr != nullptr && (m_graph[*edge_it].net_ptr->get_name() == edge_name)) {
        tag_cnt = m_signal_tag_map->get_tag_count_signal(fmt::format("{}_t", m_graph[*edge_it].net_ptr->get_name()), m_graph[*edge_it].hierarchy);
        break;
      }
    }
  }
  return tag_cnt;
}

[[nodiscard]] uint32_t DesignInstance::unique_tag_cnt_from_input_to_single_output(std::string& input_name, std::string& output_name) {
  // m_signal_tag_map-> should have a function that gets the edge for the input_name and the edge for the output_name and counts all tags originating from the input_edge that reach the output_edge
  auto input_edge = boost::edges(m_graph).first;
  auto output_edge = boost::edges(m_graph).first;

  for (auto [edge_it, edge_end] = boost::edges(m_graph); edge_it != edge_end; ++edge_it) {
    if (m_graph[*edge_it].net_ptr != nullptr && (m_graph[*edge_it].net_ptr->get_name() == input_name)) {
      input_edge = edge_it;
      // tag_cnt = m_signal_tag_map->get_tag_count_signal(fmt::format("{}_t", m_graph[*edge_it].net_ptr->get_name()), m_graph[*edge_it].hierarchy);
      break;
    }
  }
  for (auto [edge_it, edge_end] = boost::edges(m_graph); edge_it != edge_end; ++edge_it) {
    if (m_graph[*edge_it].net_ptr != nullptr && (m_graph[*edge_it].net_ptr->get_name() == output_name)) {
      output_edge = edge_it;
      // tag_cnt = m_signal_tag_map->get_tag_count_signal(fmt::format("{}_t", m_graph[*edge_it].net_ptr->get_name()), m_graph[*edge_it].hierarchy);
      break;
    }
  }
  SignalIdentifier source_signal{.name = m_graph[*input_edge].net_ptr->get_name(), .hierarchy = m_graph[*input_edge].hierarchy};
  SignalIdentifier target_signal{.name = m_graph[*output_edge].net_ptr->get_name(), .hierarchy = m_graph[*output_edge].hierarchy};

  // replace this call with new function;
  return m_signal_tag_map->get_tag_count_signal_to_signal(source_signal, target_signal);
};

}// namespace ducode
