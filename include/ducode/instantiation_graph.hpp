/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/bit.hpp>
#include <ducode/cell.hpp>
#include <ducode/cell/cell_module.hpp>
#include <ducode/design.hpp>
#include <ducode/net.hpp>
#include <ducode/port.hpp>
#include <ducode/utility/concepts.hpp>
#include <ducode/utility/parser_utility.hpp>
#include <ducode/utility/subrange.hpp>

#include <boost/container_hash/hash.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/subgraph.hpp>
#include <fmt/ranges.h>
#include <gsl/assert>
#include <range/v3/view/enumerate.hpp>
#include <spdlog/spdlog.h>
#include <vcd-parser/VCDPrinters.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace ducode {

class DesignInstance {
public:
  struct VertexInfo {
    std::string name;
    std::string type;
    std::string parent_module;
    std::vector<std::string> hierarchy;
    const Cell* cell_ptr = nullptr;// This is a non-owning pointer to a Cell.
    const Port* port_ptr = nullptr;// This is a non-owning pointer to a port.
  };

  struct EdgeInfo {
    std::string name;
    std::string parent_module;
    std::vector<std::string> hierarchy;
    const Port* source_port_ptr = nullptr;
    const Port* target_port_ptr = nullptr;
    const Net* net_ptr = nullptr;
    const VCDSignal* value_signal = nullptr;
    std::vector<VCDSignal*> tags_signal;
  };

  using instantiation_graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, VertexInfo, EdgeInfo>;

  inline const static std::set<std::string> excluded_signals = {"clk", "slaveSCLK", "sys_clk"};

  instantiation_graph m_graph;
  Design m_design;
  std::vector<std::shared_ptr<VCDFile>> m_vcd_data;

protected:
  instantiation_graph::edge_descriptor add_edge(const auto& source_vertex, const auto& target_vertex, const Port* source_port_ptr, const Port* target_port_ptr);
  std::vector<instantiation_graph::vertex_descriptor> create_module_ports_vertices(instantiation_graph::vertex_descriptor module_vertex, std::vector<std::string> hierarchy, const Cell& cell_module);
  void populate_module(instantiation_graph::vertex_descriptor module_vertex, std::vector<std::string> hierarchy, const Cell& cell_module);
  void propagate_net_pointers(const ducode::Module& top_module);
  void propagate_net_pointers_impl(const ducode::DesignInstance::instantiation_graph::edge_descriptor& current_edge, const ducode::Net* previous_edge_net, std::unordered_set<ducode::DesignInstance::instantiation_graph::edge_descriptor>& visited);

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
    auto cell_module = std::make_unique<CellModule>(top_module->get_name(), top_module->get_name(), ports, false, nlohmann::json{}, nlohmann::json{});

    VertexInfo module_vertex_prop;
    module_vertex_prop.name = top_module->get_name();
    module_vertex_prop.parent_module = "top_module";
    module_vertex_prop.hierarchy = hierarchy;
    module_vertex_prop.type = top_module->get_name();
    module_vertex_prop.cell_ptr = cell_module.get();
    auto module_vertex = boost::add_vertex(module_vertex_prop, inst.m_graph);

    inst.populate_module(module_vertex, hierarchy, *cell_module);
    inst.propagate_net_pointers(*top_module);

    for (const auto& edge: subrange(boost::edges(inst.m_graph))) {
      if (inst.m_graph[edge].net_ptr == nullptr) {
        throw std::runtime_error(fmt::format("Unrecognized net for edge: {} -> {}", inst.m_graph[edge].source_port_ptr->m_identifier, inst.m_graph[edge].target_port_ptr->m_identifier));
      }
    }

    return inst;
  }

  void add_vcd_data(const std::shared_ptr<VCDFile>& vcd_data);

  void clear_vcd_data() {
    m_vcd_data.clear();
    for (auto [ei, ei_end] = boost::edges(m_graph); ei != ei_end; ++ei) {
      m_graph[*ei].tags_signal.clear();
      m_graph[*ei].value_signal = nullptr;
    }
  }

  std::vector<std::shared_ptr<VCDFile>>& get_vcd_data() {
    return m_vcd_data;
  }

  [[nodiscard]] const VCDSignalValues* get_signal_values(const auto& edge) const {
    if (m_vcd_data.empty()) {
      throw std::runtime_error("the instantiation graph does not contain VCD data");
    }
    if (edge.get_property().value_signal == nullptr) {
      throw std::runtime_error("no values in the selected edge");
    }
    return &(m_vcd_data[0]->get_signal_values(edge.get_property().value_signal->hash));
  }

  [[nodiscard]] uint32_t number_of_empty_edges() const {
    uint32_t count = 0;

    for (const auto& edge: m_graph.m_edges) {
      if (edge.get_property().value_signal == nullptr) {
        count++;
      }
    }
    return count;
  }

  [[nodiscard]] std::vector<const VCDSignalValues*> get_tag_values(const auto& edge) const {
    std::vector<const VCDSignalValues*> result;
    for (const auto& [index, vcd_data]: ranges::views::enumerate(m_vcd_data)) {
      //todo: some edges do not have simulation values assigned to them
      if (edge.get_property().tags_signal.size() > 0) {
        result.emplace_back(&(vcd_data->get_signal_values(edge.get_property().tags_signal[index]->hash)));
      }
    }
    return result;
  }

  [[nodiscard]] const VCDSignalValues* get_signal_values(const boost::graph_traits<instantiation_graph>::edge_descriptor& edge) const {
    if (m_vcd_data.empty()) {
      throw std::runtime_error("the instantiation graph does not contain VCD data");
    }
    if (m_graph[edge].value_signal == nullptr) {
      throw std::runtime_error("no values in the selected edge");
    }
    return &(m_vcd_data[0]->get_signal_values(m_graph[edge].value_signal->hash));
  }

  [[nodiscard]] std::vector<const VCDSignalValues*> get_tag_values(const boost::graph_traits<instantiation_graph>::edge_descriptor& edge) const {
    std::vector<const VCDSignalValues*> result;
    for (const auto& [index, vcd_data]: ranges::views::enumerate(m_vcd_data)) {
      result.emplace_back(&(vcd_data->get_signal_values(m_graph[edge].tags_signal[index]->hash)));
    }
    return result;
  }

  static void write_graphviz(const instantiation_graph& graph, const std::string& filename) {
    auto file = std::ofstream(filename, std::ofstream::out);
    boost::write_graphviz(file, graph,
                          boost::make_label_writer(boost::get(&VertexInfo::name, graph)),
                          boost::make_label_writer(boost::get(&EdgeInfo::name, graph)));
  }

  void write_graphviz(const std::string& filename) {
    auto file = std::ofstream(filename, std::ofstream::out);
    boost::write_graphviz(file, m_graph,
                          boost::make_label_writer(boost::get(&VertexInfo::name, m_graph)),
                          boost::make_label_writer(boost::get(&EdgeInfo::name, m_graph)));
  }

  template<filesystem_like T>
  void write_verilog(const T& filename) {
    return write_verilog(filename.string());
  }

  void write_verilog(const std::string& filename);

  [[nodiscard]] std::deque<DesignInstance::instantiation_graph::vertex_descriptor> get_path(const auto& initial_edge, const auto& edge_comparison, const auto& edge_filter, const auto& edge_selector, const std::string& search_direction, const std::string& result_type) const;

  [[nodiscard]] auto find_tag_in_edge(auto& all_edges_filtered, auto& input_tag, auto& vcd_nr) const;

  [[nodiscard]] std::vector<std::pair<std::string, uint32_t>> count_output_tags() const;

  [[nodiscard]] static std::vector<std::pair<std::deque<DesignInstance::instantiation_graph::vertex_descriptor>, uint32_t>> count_common_path_elements(std::vector<std::deque<DesignInstance::instantiation_graph::vertex_descriptor>>& paths);

  [[nodiscard]] static bool bitwise_vcd_compare(VCDBitVector vec1, VCDBitVector vec2);

  [[nodiscard]] std::deque<DesignInstance::instantiation_graph::vertex_descriptor> least_toggled_path() {

    auto is_excluded_signal = [&](const std::string& name) {
      return std::ranges::any_of(excluded_signals, [&](const std::string& element) {
        return boost::algorithm::contains(name, element);
      });
    };

    auto edge_comparison = [&](const auto& edge1, const auto& edge2) {
      return get_signal_values(edge1)->size() < get_signal_values(edge2)->size();
    };

    auto edge_filter = [&](const auto& edge) { return !is_excluded_signal(edge.get_property().source_port_ptr->m_name) &&
                                                      !is_excluded_signal(edge.get_property().target_port_ptr->m_name); };

    const auto edge_selector = std::ranges::min_element;

    auto all_edges_filtered = m_graph.m_edges | std::ranges::views::filter(edge_filter);
    Expects(!all_edges_filtered.empty());
    // get initial edge
    const auto initial_edge = edge_selector(all_edges_filtered, edge_comparison);
    Expects(initial_edge != all_edges_filtered.end());

    const auto* const search_direction = "both";
    const auto* const result_type = "path";

    return get_path(initial_edge, edge_comparison, edge_filter, edge_selector, search_direction, result_type);
  }

  [[nodiscard]] std::deque<DesignInstance::instantiation_graph::vertex_descriptor> most_toggled_path() {
    auto is_excluded_signal = [&](const std::string& name) {
      return std::ranges::any_of(excluded_signals, [&](const std::string& element) {
        return boost::algorithm::contains(name, element);
      });
    };

    auto edge_comparison = [&](const auto& edge1, const auto& edge2) {
      return get_signal_values(edge1)->size() < get_signal_values(edge2)->size();
    };

    auto edge_filter = [&](const auto& edge) { return !is_excluded_signal(edge.get_property().source_port_ptr->m_name) &&
                                                      !is_excluded_signal(edge.get_property().target_port_ptr->m_name); };

    const auto edge_selector = std::ranges::max_element;

    auto all_edges_filtered = m_graph.m_edges | std::ranges::views::filter(edge_filter);
    Expects(!all_edges_filtered.empty());
    // get initial edge
    const auto initial_edge = edge_selector(all_edges_filtered, edge_comparison);
    Expects(initial_edge != all_edges_filtered.end());

    const auto* const search_direction = "both";
    const auto* const result_type = "path";

    return get_path(initial_edge, edge_comparison, edge_filter, edge_selector, search_direction, result_type);
  }

  [[nodiscard]] std::deque<DesignInstance::instantiation_graph::vertex_descriptor> least_tagged_path() {
    auto is_excluded_signal = [&](const std::string& name) {
      return std::ranges::any_of(excluded_signals, [&](const std::string& element) {
        return boost::algorithm::contains(name, element);
      });
    };

    auto count_tags_over_time = [&](const auto& edge) {
      std::size_t sum = 0;
      for (const auto* tag_vector: get_tag_values(edge)) {
        for (const auto& tag_value: *tag_vector) {
          auto val = std::ranges::count_if(tag_value.value.get_value_vector(), [](const auto& tag) { return tag == VCDBit::VCD_1; });
          sum += gsl::narrow<std::size_t>(val);
        }
      }
      return sum;
    };

    auto edge_comparison = [&](const auto& edge1, const auto& edge2) {
      return count_tags_over_time(edge1) < count_tags_over_time(edge2);
    };

    auto edge_filter = [&](const auto& edge) { return !is_excluded_signal(edge.get_property().source_port_ptr->m_name) &&
                                                      !is_excluded_signal(edge.get_property().target_port_ptr->m_name); };

    const auto edge_selector = std::ranges::min_element;

    auto all_edges_filtered = m_graph.m_edges | std::ranges::views::filter(edge_filter);
    Expects(!all_edges_filtered.empty());
    // get initial edge
    const auto initial_edge = edge_selector(all_edges_filtered, edge_comparison);
    Expects(initial_edge != all_edges_filtered.end());

    const auto* const search_direction = "both";
    const auto* const result_type = "path";

    return get_path(initial_edge, edge_comparison, edge_filter, edge_selector, search_direction, result_type);
  }

  [[nodiscard]] std::deque<DesignInstance::instantiation_graph::vertex_descriptor> most_tagged_path() {
    auto is_excluded_signal = [&](const std::string& name) {
      return std::ranges::any_of(excluded_signals, [&](const std::string& element) {
        return boost::algorithm::contains(name, element);
      });
    };

    auto count_tags_over_time = [&](const auto& edge) {
      std::size_t sum = 0;
      for (const auto* tag_vector: get_tag_values(edge)) {
        for (const auto& tag_value: *tag_vector) {
          auto val = std::ranges::count_if(tag_value.value.get_value_vector(), [](const auto& tag) { return tag == VCDBit::VCD_1; });
          sum += gsl::narrow<std::size_t>(val);
        }
      }
      return sum;
    };

    auto edge_comparison = [&](const auto& edge1, const auto& edge2) {
      return count_tags_over_time(edge1) < count_tags_over_time(edge2);
    };

    auto edge_filter = [&](const auto& edge) { return !is_excluded_signal(edge.get_property().source_port_ptr->m_name) &&
                                                      !is_excluded_signal(edge.get_property().target_port_ptr->m_name); };

    const auto edge_selector = std::ranges::max_element;

    auto all_edges_filtered = m_graph.m_edges | std::ranges::views::filter(edge_filter);
    Expects(!all_edges_filtered.empty());
    // get initial edge
    const auto initial_edge = edge_selector(all_edges_filtered, edge_comparison);
    Expects(initial_edge != all_edges_filtered.end());

    const auto* const search_direction = "both";
    const auto* const result_type = "path";

    return get_path(initial_edge, edge_comparison, edge_filter, edge_selector, search_direction, result_type);
  }

  [[nodiscard]] bool signal_contains_x_values(auto& edge) {
    auto signal_values = get_signal_values(edge);
    for (const auto& signal_value: *signal_values) {
      if (signal_value.value.get_type() == VCDValueType::VECTOR) {
        for (const auto& signal_bit: signal_value.value.get_value_vector()) {
          if (signal_bit == VCDBit::VCD_X) {
            return true;
            break;
          }
        }
      } else if (signal_value.value.get_type() == VCDValueType::SCALAR) {
        if (signal_value.value.get_value_bit() == VCDBit::VCD_X) {
          return true;
        }
      }
    }
    return false;
  }

  [[nodiscard]] bool signal_tags_contains_x_values(auto& edge) {
    for (const auto& tag_value: get_tag_values(edge)) {
      for (const auto& tag_val: *tag_value) {
        if (tag_val.value.get_type() == VCDValueType::VECTOR) {
          for (const auto& signal_bit: tag_val.value.get_value_vector()) {
            if (signal_bit == VCDBit::VCD_X) {
              return true;
              break;
            }
          }
        } else if (tag_val.value.get_type() == VCDValueType::SCALAR) {
          if (tag_val.value.get_value_bit() == VCDBit::VCD_X) {
            return true;
          }
        }
      }
    }
    return false;
  }

  [[nodiscard]] std::deque<DesignInstance::instantiation_graph::vertex_descriptor> x_tag_path() {
    auto edge_select_condition = [&](const auto& edge) {
      return signal_tags_contains_x_values(edge);
    };

    auto edge_filter = [&](const auto& edge) { return static_cast<bool>(edge.get_property().value_signal != nullptr); };//create all edges view

    const auto edge_selector = [&](auto& all_edges_filtered, const auto& select_condition) {
      for (auto it = all_edges_filtered.begin(); it != all_edges_filtered.end(); ++it) {
        if (select_condition(*it)) {
          return it;
        }
      }
      return all_edges_filtered.end();
    };

    auto all_edges_filtered = m_graph.m_edges | std::ranges::views::filter(edge_filter);
    Expects(!all_edges_filtered.empty());
    // get initial edge
    const auto initial_edge = edge_selector(all_edges_filtered, edge_select_condition);
    if (initial_edge == all_edges_filtered.end()) {
      return {};
    }

    const auto* const search_direction = "both";
    const auto* const result_type = "path";

    return get_path(initial_edge, edge_select_condition, edge_filter, edge_selector, search_direction, result_type);
  }

  /**
   * @brief This function returns the path that follows a single tag from input to an output
  */
  [[nodiscard]] std::deque<DesignInstance::instantiation_graph::vertex_descriptor> tag_path_from_input(auto& input_name, auto& tag_vcd_time) {
    //use the initial edge and the tag_vcd_time to find the vcd_file and tag_value for the wanted tag
    //loop through the tags of the initial edge and find the vcd file in which the initial edge was tagged for the specified time
    auto tag = [&](const auto& edge) {
      auto tag_values = get_tag_values(edge);
      for (const auto& [index, tag_value]: ranges::views::enumerate(tag_values)) {
        for (const auto& tag_val: *tag_value) {
          if (tag_val.time == tag_vcd_time) {
            return std::make_pair(static_cast<uint32_t>(index), tag_val.value);
          }
        }
      }
      return std::make_pair(static_cast<uint32_t>(0), VCDValue{});
    };

    auto is_excluded_signal = [&](const std::string& name) {
      return std::ranges::any_of(excluded_signals, [&](const std::string& element) {
        return boost::algorithm::contains(name, element);
      });
    };
    auto edge_filter = [&](const auto& edge) { return !is_excluded_signal(edge.get_property().source_port_ptr->m_name) &&
                                                      !is_excluded_signal(edge.get_property().target_port_ptr->m_name) &&
                                                      static_cast<bool>(!edge.get_property().tags_signal.empty()); };
    // auto edge_filter = [&](const auto& edge) { return static_cast<bool>(!edge.get_property().tags_signal.empty()); };//get all edges with at least one tag

    auto all_edges_filtered = m_graph.m_edges | std::ranges::views::filter(edge_filter);
    Expects(!all_edges_filtered.empty());

    auto get_initial_edge = [&](auto& filtered_edges) {
      for (auto it = filtered_edges.begin(); it != filtered_edges.end(); ++it) {
        if (it->get_property().source_port_ptr->m_name == input_name) {
          return it;
        }
      }
      return filtered_edges.end();
    };

    // get initial edge
    const auto initial_edge = get_initial_edge(all_edges_filtered);
    if (initial_edge == all_edges_filtered.end()) {
      return {};
    }

    auto input_tag = tag(*initial_edge);
    if (input_tag.second.get_type() == VCDValueType::EMPTY) {
      return {};
    }
    // select_condition is a lambda that returns true if the edge contains the wanted tag value (wanted tag value is the tag that was injected for the input signal at the specified time)
    auto edge_select_condition = [&](const auto& edge) {
      const auto& tag_value = get_tag_values(edge);
      const auto& tag_vector = *tag_value[input_tag.first];
      return std::any_of(tag_vector.begin(), tag_vector.end(), [&](const auto& tag_val) {
        if (tag_val.value.get_type() == VCDValueType::VECTOR && input_tag.second.get_type() == VCDValueType::VECTOR) {
          return bitwise_vcd_compare(tag_val.value.get_value_vector(), input_tag.second.get_value_vector());
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

    return get_path(initial_edge, edge_select_condition, edge_filter, edge_selector, search_direction, result_type);
  };
};


}// namespace ducode

template<>
struct std::hash<ducode::DesignInstance::instantiation_graph::edge_descriptor> {
  std::size_t operator()(const ducode::DesignInstance::instantiation_graph::edge_descriptor& edge) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, edge.m_source);
    boost::hash_combine(seed, edge.m_target);
    return seed;
  }
};

#include "detail/instantiation_graph_inl.hpp"