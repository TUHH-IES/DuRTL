/* SPDX-License-Identifier: Apache-2.0 */

#include <ducode/bit.hpp>
#include <ducode/cell.hpp>
#include <ducode/cell/all_cells.hpp>// NOLINT(misc-include-cleaner)
#include <ducode/detail/module_graph.hpp>
#include <ducode/ids.hpp>
#include <ducode/module.hpp>
#include <ducode/net.hpp>
#include <ducode/port.hpp>
#include <ducode/types.hpp>
#include <ducode/utility/export_connection.hpp>
#include <ducode/utility/graph.hpp>
#include <ducode/utility/legalize_identifier.hpp>
#include <ducode/utility/subrange.hpp>

#include <boost/algorithm/string/join.hpp>
#include <boost/graph/detail/adjacency_list.hpp>
#include <fmt/core.h>
#include <fmt/ranges.h>// NOLINT(misc-include-cleaner)
#include <gsl/assert>
#include <nlohmann/json_fwd.hpp>
#include <range/v3/view/concat.hpp>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace ducode {

void Module::populate_graph() {
  auto vectors_intersection = [](const fast_bit_container& bits1, const fast_bit_container& bits2) -> bool {
    if (bits1.empty() || bits2.empty()) {
      throw std::runtime_error("Empty bits!");
    }

    return std::ranges::any_of(bits1, [&](const Bit& bit) { return bits2.contains(bit); });
  };

  std::unordered_multimap<std::size_t, ModuleGraph::vertex_descriptor> net_signatures;
  std::unordered_multimap<std::size_t, ModuleGraph::vertex_descriptor> module_port_signatures;

  // adding all the nets; ports are directly connected to matching nets, if any
  for (const Net& net: m_nets) {
    auto port_it = std::ranges::find_if(m_ports, [&](const Port& port) { return port.m_bits_signature == net.get_bits_signature() && port.m_name == net.get_name(); });
    if (port_it == m_ports.end()) {
      VertexInfo module_vertex_prop;
      module_vertex_prop.type = VertexType::kNet;
      module_vertex_prop.net_ptr = &net;
      module_vertex_prop.label = fmt::format("net -> {} ({})", net.get_name(), net.get_bits());
      auto net_vertex = boost::add_vertex(module_vertex_prop, m_module_graph);
      net_signatures.emplace(net.get_bits_signature(), net_vertex);
    } else {
      VertexInfo module_vertex_prop;
      module_vertex_prop.type = VertexType::kNet;
      module_vertex_prop.port_type = VertexType::kModulePort;
      module_vertex_prop.net_ptr = &net;
      module_vertex_prop.port_ptr = &(*port_it);
      module_vertex_prop.label = fmt::format("port net -> {} ({})", net.get_name(), net.get_bits());
      auto port_vertex = boost::add_vertex(module_vertex_prop, m_module_graph);
      module_port_signatures.emplace(net.get_bits_signature(), port_vertex);

      auto net_range = ducode::subrange(net_signatures.equal_range(net.get_bits_signature()));
      for (auto [net_signature, net_descriptor]: net_range) {
        if (port_it->m_direction == Port::Direction::input) {
          boost::add_edge(port_vertex, net_descriptor, m_module_graph);
        } else if (port_it->m_direction == Port::Direction::output || port_it->m_direction == Port::Direction::inout) {
          boost::add_edge(net_descriptor, port_vertex, m_module_graph);
        } else {
          throw std::runtime_error("Case not managed.");
        }
      }
    }
  }

  // adding all the cells and connecting them to the existing nets or to new nets
  std::unordered_multimap<std::size_t, ModuleGraph::vertex_descriptor> cell_port_signatures;
  for (const Cell& cell: m_cells) {
    VertexInfo vertex_prop;
    vertex_prop.type = VertexType::kCell;
    vertex_prop.cell_ptr = &cell;
    vertex_prop.label = fmt::format("cell -> {}", cell.get_name());
    auto cell_vertex = boost::add_vertex(vertex_prop, m_module_graph);

    for (const Port& port: cell.get_ports()) {
      std::vector<ModuleGraph::vertex_descriptor> to_be_connected;

      if (net_signatures.contains(port.m_bits_signature)) {
        to_be_connected.emplace_back(net_signatures.find(port.m_bits_signature)->second);
      } else if (module_port_signatures.contains(port.m_bits_signature)) {
        to_be_connected.emplace_back(module_port_signatures.find(port.m_bits_signature)->second);
      }

      for (const auto& vertex: to_be_connected) {
        if (port.m_direction == Port::Direction::input) {
          boost::add_edge(vertex, cell_vertex, m_module_graph);
        } else if (port.m_direction == Port::Direction::output || port.m_direction == Port::Direction::inout) {
          boost::add_edge(cell_vertex, vertex, m_module_graph);
        } else {
          throw std::runtime_error("Case not managed.");
        }
      }

      if (to_be_connected.empty() && !port.m_bits.empty()) {
        VertexInfo module_vertex_prop;
        module_vertex_prop.type = VertexType::kCellPort;
        module_vertex_prop.cell_ptr = &cell;
        module_vertex_prop.port_ptr = &port;
        module_vertex_prop.label = fmt::format("cell port -> {}.{} ({})", cell.get_name(), port.m_name, port.m_bits);
        auto port_vertex = boost::add_vertex(module_vertex_prop, m_module_graph);
        cell_port_signatures.emplace(port.m_bits_signature, port_vertex);
        if (port.m_direction == Port::Direction::input) {
          boost::add_edge(port_vertex, cell_vertex, m_module_graph);
        } else if (port.m_direction == Port::Direction::output || port.m_direction == Port::Direction::inout) {
          boost::add_edge(cell_vertex, port_vertex, m_module_graph);
        } else {
          throw std::runtime_error("Case not managed.");
        }
      }
    }
  }

  // connecting remaining elements
  auto target_vertices = subrange(boost::vertices(m_module_graph));
  for (const auto& target_vertex: target_vertices) {
    if (m_module_graph[target_vertex].type == VertexType::kCell) {
      continue;
    }

    if (m_module_graph[target_vertex].port_type == VertexType::kModulePort && m_module_graph[target_vertex].port_ptr->m_direction == Port::Direction::input) {
      continue;
    }

    // if this node has already a source
    const auto in_degree = boost::in_degree(target_vertex, m_module_graph);
    if (in_degree != 0) {
      continue;
    }

    const auto* target_port_ptr = m_module_graph[target_vertex].port_ptr;
    const auto* target_net_ptr = m_module_graph[target_vertex].net_ptr;

    std::size_t target_bits_signature = 0;
    fast_bit_container target_bits;

    Ensures(target_port_ptr != nullptr || target_net_ptr != nullptr);

    if (target_net_ptr != nullptr) {
      if (target_net_ptr->contains_only_constants()) {
        continue;
      }
      target_bits_signature = target_net_ptr->get_bits_signature();
      target_bits = target_net_ptr->get_fast_search_bits();
    } else {
      if (target_port_ptr->contains_only_constants()) {
        continue;
      }
      target_bits_signature = target_port_ptr->m_bits_signature;
      target_bits = target_port_ptr->m_fast_search_bits;
    }

    std::vector<ModuleGraph::vertex_descriptor> to_be_connected;

    auto cell_ports_range = subrange(cell_port_signatures.equal_range(target_bits_signature));
    auto nets_range = subrange(net_signatures.equal_range(target_bits_signature));
    auto module_ports_range = subrange(module_port_signatures.equal_range(target_bits_signature));
    auto range = ranges::concat_view(module_ports_range, nets_range, cell_ports_range);

    for (const auto& [source_signature, source_vertex]: range) {
      if (source_vertex == target_vertex) {
        continue;
      }
      if (m_module_graph[source_vertex].type == VertexType::kCellPort && m_module_graph[source_vertex].port_ptr->m_direction == Port::Direction::input) {
        continue;
      }
      if (source_reachable(m_module_graph, source_vertex, target_vertex)) {
        continue;
      }

      to_be_connected.emplace_back(source_vertex);
      target_bits.clear();
      break;
    }

    for (const auto& [source_bits_signature, source_vertex]: module_port_signatures) {
      if (target_bits.empty()) {
        break;
      }
      if (source_vertex == target_vertex) {
        continue;
      }
      const auto* source_net_ptr = m_module_graph[source_vertex].net_ptr;
      Ensures(source_net_ptr != nullptr);
      if (source_net_ptr->contains_only_constants()) {
        continue;
      }
      if (source_reachable(m_module_graph, source_vertex, target_vertex)) {
        continue;
      }

      const fast_bit_container& source_bits = source_net_ptr->get_fast_search_bits();
      if (source_bits_signature == target_bits_signature || vectors_intersection(source_bits, target_bits)) {
        to_be_connected.emplace_back(source_vertex);
        for (const auto& element: source_bits) {
          target_bits.erase(element);
        }
      }
    }

    for (const auto& [source_bits_signature, source_vertex]: net_signatures) {
      if (target_bits.empty()) {
        break;
      }
      if (source_vertex == target_vertex) {
        continue;
      }
      const auto* source_net_ptr = m_module_graph[source_vertex].net_ptr;
      Ensures(source_net_ptr != nullptr);

      if (source_net_ptr->contains_only_constants()) {
        continue;
      }
      if (source_reachable(m_module_graph, source_vertex, target_vertex)) {
        continue;
      }

      const fast_bit_container& source_bits = source_net_ptr->get_fast_search_bits();
      if (source_bits_signature == target_bits_signature || vectors_intersection(source_bits, target_bits)) {
        to_be_connected.emplace_back(source_vertex);
        for (const auto& element: source_bits) {
          target_bits.erase(element);
        }
      }
    }

    for (const auto& [source_bits_signature, source_vertex]: cell_port_signatures) {
      if (target_bits.empty()) {
        break;
      }
      if (source_vertex == target_vertex) {
        continue;
      }
      if (m_module_graph[source_vertex].port_ptr->m_direction == Port::Direction::input) {
        continue;
      }
      const auto* source_port_ptr = m_module_graph[source_vertex].port_ptr;
      Ensures(source_port_ptr != nullptr);

      if (source_port_ptr->contains_only_constants()) {
        continue;
      }
      if (source_reachable(m_module_graph, source_vertex, target_vertex)) {
        continue;
      }

      const fast_bit_container& source_bits = source_port_ptr->m_fast_search_bits;
      if (source_bits_signature == target_bits_signature || vectors_intersection(source_bits, target_bits)) {
        to_be_connected.emplace_back(source_vertex);
        for (const auto& element: source_bits) {
          target_bits.erase(element);
        }
      }
    }

    for (const auto& source_vertex: to_be_connected) {
      boost::add_edge(source_vertex, target_vertex, m_module_graph);
    }
  }
}

[[nodiscard]] std::string Module::export_verilog_header(const nlohmann::json& params) const {
  Expects(params.contains("ift"));
  const bool ift = params["ift"].get<bool>();

  Expects(params.contains("tag_size"));
  const uint32_t tag_size = params["tag_size"].get<uint32_t>();

  auto input_names = get_input_names();
  auto output_names = get_output_names();
  auto inout_names = get_inout_names();

  if (ift) {
    input_names.clear();
    for (const auto& name: get_input_names()) {
      input_names.emplace_back(legalize_identifier(name));
      input_names.emplace_back(fmt::format("{}{}", legalize_identifier(name), ift_tag));
    }
    output_names.clear();
    for (const auto& name: get_output_names()) {
      output_names.emplace_back(legalize_identifier(name));
      output_names.emplace_back(fmt::format("{}{}", legalize_identifier(name), ift_tag));
    }
    inout_names.clear();
    for (const auto& name: get_inout_names()) {
      inout_names.emplace_back(legalize_identifier(name));
      inout_names.emplace_back(fmt::format("{}{}", legalize_identifier(name), ift_tag));
    }
  }
  auto all_ports = ranges::views::concat(input_names, output_names, inout_names);
  auto all_ports_string = boost::algorithm::join(all_ports, ", ");

  std::stringstream result;

  result << fmt::format("module {}({});\n", legalize_identifier(m_name), all_ports_string);

  for (const auto& port: m_ports) {
    std::string direction_string;
    if (port.m_direction == Port::Direction::input) {
      direction_string = "input";
    } else if (port.m_direction == Port::Direction::output) {
      direction_string = "output";
    } else if (port.m_direction == Port::Direction::inout) {
      direction_string = "inout";
    } else {
      throw std::runtime_error("port direction not recognized");
    }

    std::string width_string = port.m_bits.size() > 1 ? fmt::format("[{}:0]", port.m_bits.size() - 1) : "";

    result << fmt::format("{} {} {};\n", direction_string, width_string, legalize_identifier(port.m_name));
    if (ift) {
      result << fmt::format("{} [{}:0] {}{};\n", direction_string, tag_size - 1, legalize_identifier(port.m_name), ift_tag);
    }
  }

  return result.str();
}

std::string Module::export_verilog(const nlohmann::json& params) const {
  bool ift = false;
  if (params.contains("ift")) {
    ift = params["ift"].get<bool>();
  }

  std::stringstream result;

  result << export_verilog_header(params);

  for (auto vertex: subrange(boost::vertices(m_module_graph))) {
    if (m_module_graph[vertex].type == VertexType::kNet) {
      Expects(m_module_graph[vertex].net_ptr != nullptr);
      const auto* net_ptr = m_module_graph[vertex].net_ptr;
      auto net_params = params;
      auto net_sources = subrange(boost::inv_adjacent_vertices(vertex, m_module_graph));
      for (const auto& source: net_sources) {
        if (m_module_graph[source].type == VertexType::kCell && m_module_graph[source].cell_ptr->has_register()) {
          net_params["is_register"] = true;
        }
      }
      result << net_ptr->export_verilog(net_params);
    } else if (m_module_graph[vertex].type == VertexType::kCellPort) {
      Expects(m_module_graph[vertex].port_ptr != nullptr && m_module_graph[vertex].cell_ptr != nullptr && m_module_graph[vertex].net_ptr == nullptr);
      const auto* port_ptr = m_module_graph[vertex].port_ptr;
      auto port_params = params;
      auto port_sources = subrange(boost::inv_adjacent_vertices(vertex, m_module_graph));
      for (const auto& source: port_sources) {
        if (m_module_graph[source].type == VertexType::kCell && m_module_graph[source].cell_ptr->has_register()) {
          port_params["is_register"] = true;
        }
      }
      result << port_ptr->export_verilog(port_params);
    }
  }

  for (auto vertex: subrange(boost::vertices(m_module_graph))) {
    std::vector<ModuleGraph::vertex_descriptor> sources;
    std::string target_name;
    bit_container target_bits;

    if (m_module_graph[vertex].type == VertexType::kNet) {
      Expects(m_module_graph[vertex].net_ptr != nullptr);
      const auto* net_ptr = m_module_graph[vertex].net_ptr;
      target_name = legalize_identifier(net_ptr->get_name());
      target_bits = net_ptr->get_bits();
      auto net_sources = subrange(boost::inv_adjacent_vertices(vertex, m_module_graph));
      for (const auto& source: net_sources) {
        sources.emplace_back(source);
      }
    } else if (m_module_graph[vertex].type == VertexType::kCellPort) {
      Expects(m_module_graph[vertex].port_ptr != nullptr && m_module_graph[vertex].cell_ptr != nullptr && m_module_graph[vertex].net_ptr == nullptr);
      const auto* port_ptr = m_module_graph[vertex].port_ptr;
      target_name = legalize_identifier(port_ptr->m_identifier);
      target_bits = port_ptr->m_bits;
      auto port_sources = subrange(boost::inv_adjacent_vertices(vertex, m_module_graph));
      for (const auto& source: port_sources) {
        sources.emplace_back(source);
      }
    }

    std::vector<std::string> ift_source_names;
    for (const auto& source: sources) {
      std::string source_name;
      bit_container source_bits;
      if (m_module_graph[source].type == VertexType::kCellPort) {
        const auto* port_ptr = m_module_graph[source].port_ptr;
        Ensures(port_ptr != nullptr);
        source_name = legalize_identifier(port_ptr->m_identifier);
        source_bits = port_ptr->m_bits;
      } else if (m_module_graph[source].type == VertexType::kNet) {
        const auto* net_ptr = m_module_graph[source].net_ptr;
        Ensures(net_ptr != nullptr);
        source_name = legalize_identifier(net_ptr->get_name());
        source_bits = net_ptr->get_bits();
      } else {
        continue;
      }

      result << export_connection(source_name, source_bits, target_name, target_bits);
      if (ift) {
        ift_source_names.emplace_back(source_name + ift_tag);
      }
    }

    if (!ift_source_names.empty()) {
      result << fmt::format(" assign {}{} = {};\n", target_name, ift_tag, boost::join(ift_source_names, " | "));
    }
  }

  for (auto vertex: subrange(boost::vertices(m_module_graph))) {
    if (m_module_graph[vertex].type == VertexType::kCell) {
      Expects(m_module_graph[vertex].cell_ptr != nullptr && m_module_graph[vertex].port_ptr == nullptr);
      const auto* cell_ptr = m_module_graph[vertex].cell_ptr;
      spdlog::debug(cell_ptr->get_type());

      auto cell_params = params;
      std::unordered_map<std::size_t, std::string> identifiers_map;
      auto view1 = subrange(boost::adjacent_vertices(vertex, m_module_graph));
      auto view2 = subrange(boost::inv_adjacent_vertices(vertex, m_module_graph));
      auto adjacent_vertices_range = ranges::views::concat(view1, view2);

      for (const auto& adjacent_vertex: adjacent_vertices_range) {
        if (m_module_graph[adjacent_vertex].type == VertexType::kNet || m_module_graph[adjacent_vertex].type == VertexType::kModulePort) {
          Expects(m_module_graph[adjacent_vertex].net_ptr != nullptr);
          const auto* net_ptr = m_module_graph[adjacent_vertex].net_ptr;
          identifiers_map.emplace(net_ptr->get_bits_signature(), legalize_identifier(net_ptr->get_name()));
        } else if (m_module_graph[adjacent_vertex].type == VertexType::kCellPort) {
          Expects(m_module_graph[adjacent_vertex].port_ptr != nullptr);
          const auto* port_ptr = m_module_graph[adjacent_vertex].port_ptr;
          identifiers_map.emplace(port_ptr->m_bits_signature, legalize_identifier(port_ptr->m_identifier));
        }
      }

      cell_params["identifiers_map"] = identifiers_map;
      result << cell_ptr->export_verilog(cell_params);
    }
  }

  result << "endmodule" << '\n';

  return result.str();
}

[[nodiscard]] bool Module::check_clock_signal_consistency() const {
  const Net* clk_net = nullptr;
  for (auto vertex: subrange(boost::vertices(m_module_graph))) {
    if (m_module_graph[vertex].type == VertexType::kCell) {
      Expects(m_module_graph[vertex].cell_ptr != nullptr && m_module_graph[vertex].port_ptr == nullptr);
      const auto* cell_ptr = m_module_graph[vertex].cell_ptr;

      if (cell_ptr->has_register()) {
        spdlog::debug(cell_ptr->get_type());
        if (!cell_ptr->has_port("CLK")) {
          continue;
        }
        auto clk_port = cell_ptr->get_port("CLK");

        std::unordered_map<std::size_t, std::string> identifiers_map;
        auto view1 = subrange(boost::adjacent_vertices(vertex, m_module_graph));
        auto view2 = subrange(boost::inv_adjacent_vertices(vertex, m_module_graph));
        auto adjacent_vertices_range = ranges::views::concat(view1, view2);

        for (const auto& adjacent_vertex: adjacent_vertices_range) {
          if (m_module_graph[adjacent_vertex].type == VertexType::kNet || m_module_graph[adjacent_vertex].type == VertexType::kModulePort) {
            Expects(m_module_graph[adjacent_vertex].net_ptr != nullptr);
            const auto* net_ptr = m_module_graph[adjacent_vertex].net_ptr;
            if (net_ptr->get_bits_signature() == clk_port.m_bits_signature) {
              if (clk_net == nullptr) {
                clk_net = net_ptr;
                break;
              }
              if (clk_net != net_ptr) {
                spdlog::error("In module {}, multiple clock signals detected: {} and {}", m_name, clk_net->get_name(), net_ptr->get_name());
                return false;
              }
            }
          }
        }
      }
    }
  }
  return true;
}

[[nodiscard]] bool Module::check_clock_rising_edge_consistency() const {
  auto [vertex_begin, vertex_end] = boost::vertices(m_module_graph);

  auto check_rising_edge = [&](const auto& vertex) {
    if (m_module_graph[vertex].type == VertexType::kCell) {
      Expects(m_module_graph[vertex].cell_ptr != nullptr && m_module_graph[vertex].port_ptr == nullptr);
      const auto* cell_ptr = m_module_graph[vertex].cell_ptr;

      if (cell_ptr->has_register()) {
        spdlog::debug(cell_ptr->get_type());
        if (!cell_ptr->has_port("CLK")) {
          return true;
        }

        if (cell_ptr->get_parameters().contains("CLK_POLARITY")) {
          const auto clk_polarity = cell_ptr->get_parameters().at("CLK_POLARITY").template get<std::string>();
          if (clk_polarity != "1") {
            spdlog::error("In module {}, cell {} has a clock port not triggered on rising edge", m_name, cell_ptr->get_name());
            return false;
          }
        }
      }
    }
    return true;
  };

  return std::ranges::all_of(vertex_begin, vertex_end, check_rising_edge);
}

// NOLINTBEGIN(misc-include-cleaner)

void Module::add_cell(const std::unordered_set<std::string>& modules, const std::string& name, const std::string& type, std::vector<ducode::Port>& ports, bool hidden, const nlohmann::json& parameters, const nlohmann::json& attributes) {
  std::set<std::string> basic_binary_gates = {"$add", "$sub", "$mul", "$div", "$eq", "$eqx", "$ge", "$gt", "$le", "$lt", "$mod", "$ne", "$nex", "$pow", "$shl", "$shr", "$sshl", "$sshr"};
  std::set<std::string> basic_unary_gates = {"$not", "$logic_not", "$pos", "$neg"};


  //////////// BINARY OPERATORS ///////////////
  if (type == "$and") {
    m_cells.emplace<CellAnd>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$or") {
    m_cells.emplace<CellOr>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$xor") {
    m_cells.emplace<CellXor>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$xnor") {
    m_cells.emplace<CellXnor>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$shift") {
    m_cells.emplace<CellShift>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$shiftx") {
    m_cells.emplace<CellShiftx>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$logic_and") {
    m_cells.emplace<CellLogicAnd>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$logic_or") {
    m_cells.emplace<CellLogicOr>(name, type, ports, hidden, parameters, attributes);
  } else if (basic_binary_gates.contains(type)) {
    m_cells.emplace<CellBasicGateBinary>(name, type, ports, hidden, parameters, attributes);
    //////////// UNARY OPERATORS ///////////////
  } else if (basic_unary_gates.contains(type)) {
    m_cells.emplace<CellBasicGateUnary>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$reduce_and") {
    m_cells.emplace<CellReduceAnd>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$reduce_or" || type == "$reduce_bool") {//functionally the same as $reduce_or according to yosys manual; so a $reduce_bool type creates a 'CellReduceOr'
    m_cells.emplace<CellReduceOr>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$reduce_xor") {
    m_cells.emplace<CellReduceXor>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$reduce_xnor") {
    m_cells.emplace<CellReduceXnor>(name, type, ports, hidden, parameters, attributes);
    //////////// MULTIPLEXER ///////////////
  } else if (type == "$mux") {
    m_cells.emplace<CellMux>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$pmux") {
    m_cells.emplace<CellPMux>(name, type, ports, hidden, parameters, attributes);
    //////////// REGISTERS (DFF) ///////////////
  } else if (type == "$sr") {
    m_cells.emplace<CellSR>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$dff") {
    m_cells.emplace<CellDFF>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$adff") {
    m_cells.emplace<CellADFF>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$sdff") {
    m_cells.emplace<CellSDFF>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$aldff") {
    m_cells.emplace<CellALDFF>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$dffsr") {
    m_cells.emplace<CellDFFSR>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$dffe") {
    m_cells.emplace<CellDFFE>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$adffe") {
    m_cells.emplace<CellADFFE>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$sdffe") {
    m_cells.emplace<CellSDFFE>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$aldffe") {
    m_cells.emplace<CellALDFFE>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$dffsre") {
    m_cells.emplace<CellDFFSRE>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$dlatch") {
    m_cells.emplace<CellDLatch>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$adlatch") {
    m_cells.emplace<CellADLatch>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$dlatchsr") {
    m_cells.emplace<CellDLatchSR>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$print") {
    m_cells.emplace<Print>(name, type, ports, hidden, parameters, attributes);
  } else if (type == "$mem_v2") {
    m_cells.emplace<MemV2>(name, type, ports, hidden, parameters, attributes);
  } else {
    if (modules.contains(type)) {
      spdlog::debug("CellType: Module ({})", type);
      m_cells.emplace<CellModule>(legalize_identifier(name), type, ports, hidden, parameters, attributes);
    } else {
      throw std::runtime_error(fmt::format("The type is not a Cell or a Module: {}", type));
    }
  }
}

// NOLINTEND(misc-include-cleaner)

}// namespace ducode
