/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/cell.hpp>
#include <ducode/detail/module_graph.hpp>
#include <ducode/net.hpp>
#include <ducode/port.hpp>
#include <ducode/types.hpp>
#include <ducode/utility/export_connection.hpp>
#include <ducode/utility/graph.hpp>
#include <ducode/utility/subrange.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/poly_collection/base_collection.hpp>
#include <fmt/ranges.h>
#include <gsl/assert>
#include <gsl/narrow>
#include <nlohmann/json.hpp>
#include <range/v3/view/concat.hpp>
#include <range/v3/view/enumerate.hpp>
#include <spdlog/spdlog.h>
#include <vcd-parser/VCDFile.hpp>

#include <algorithm>
#include <cstdint>
#include <map>
#include <ostream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace ducode {

/**
 * @brief Represents a module in a design.
 * 
 * The `Module` class encapsulates the properties and behaviors of a module in a design.
 * It contains information such as the module name, source information, ports, cells, nets, and the module graph.
 * 
 * Modules can be copied and moved, and provide methods to access and manipulate their ports, cells, and nets.
 * They also provide methods to populate the module graph, which represents the connectivity between ports, cells, and nets.
 */
class Module {
  std::string m_name;
  std::string m_src_name;
  uint32_t m_src_line = 0;
  std::vector<Net> m_nets;
  boost::base_collection<Cell> m_cells;
  std::vector<Port> m_ports;
  ModuleGraph m_module_graph;

public:
  /**
   * @brief Constructs a Module object with the given name.
   * 
   * @param name The name of the module.
   */
  explicit Module(std::string name) : m_name(std::move(name)) {}

  /**
   * @brief Constructs a new Module object by copying another Module.
   *
   * @param other The Module object to be copied.
   */
  Module(const Module& other) : m_name(other.m_name),
                                m_src_name(other.m_src_name),
                                m_src_line(other.m_src_line),
                                m_nets(other.m_nets),
                                m_cells(other.m_cells),
                                m_ports(other.m_ports) {
    spdlog::info("Module copied");
    populate_graph();
  }

  Module(Module&&) noexcept = default;

  /**
   * @brief Assignment operator for the Module class.
   * 
   * This operator assigns the values of another Module object to the current object.
   * It performs a deep copy of the member variables and calls the populate_graph() function.
   * 
   * @param other The Module object to be assigned.
   * @return A reference to the current Module object after assignment.
   */
  Module& operator=(const Module& other) {
    if (this == &other) {
      return *this;
    }

    m_name = other.m_name;
    m_src_name = other.m_src_name;
    m_src_line = other.m_src_line;
    m_nets = other.m_nets;
    m_cells = other.m_cells;
    m_ports = other.m_ports;
    spdlog::info("Module copied");
    populate_graph();
    return *this;
  }

  /**
   * @brief Assignment operator for moving a Module object.
   * 
   * This assignment operator allows for moving the contents of one Module object into another.
   * It is declared as noexcept to indicate that it does not throw any exceptions.
   * The default keyword indicates that the compiler-generated implementation should be used.
   * 
   * @param other The Module object to move from.
   * @return A reference to the modified Module object.
   */
  Module& operator=(Module&& other) noexcept = default;
  /**
   * @brief Destructor for the Module class.
   * 
   * This destructor is defaulted, meaning it will use the default behavior provided by the compiler.
   * It is responsible for cleaning up any resources held by the Module object.
   */
  ~Module() = default;

  /**
   * @brief Get the ports of the module.
   * 
   * This function returns a const reference to the vector of ports in the module.
   * 
   * @return const std::vector<Port>& The vector of ports.
   */
  [[nodiscard]] const std::vector<Port>& get_ports() const {
    return m_ports;
  }

  /**
   * @brief Retrieves the port with the specified name.
   * 
   * This function searches for a port in the module's list of ports that has the given name.
   * If a matching port is found, it is returned.
   * If no port with the specified name is found, a std::runtime_error is thrown.
   * 
   * @param port_name The name of the port to retrieve.
   * @return const Port& A reference to the port with the specified name.
   * @throws std::runtime_error if no port with the specified name is found.
   */
  [[nodiscard]] const Port& get_port(std::string port_name) const {
    auto port_it = std::ranges::find_if(m_ports, [&](const Port& port) { return port.m_name == port_name; });
    if (port_it == m_ports.end()) {
      throw std::runtime_error("Port name not found!");
    }
    return *port_it;
  }

  /**
   * Adds a port to the module.
   *
   * @param port_name The name of the port.
   * @param port_direction The direction of the port.
   * @param bits The bits associated with the port.
   */
  void add_port(std::string port_name, Port::Direction port_direction, const bit_container& bits) {
    m_ports.emplace_back(port_name, fmt::format("{}_{}", legalize_identifier(m_name), port_name), port_direction, bits, false);
  }

  /**
   * @brief Returns a reference to the collection of cells.
   *
   * This function returns a const reference to the collection of cells in the module.
   * The collection is of type `boost::base_collection<Cell>`.
   *
   * @return A const reference to the collection of cells.
   */
  [[nodiscard]] const boost::base_collection<Cell>& get_cells() const {
    return m_cells;
  }

  /**
   * @brief Adds a cell to the module.
   *
   * This function adds a cell to the module with the specified parameters.
   *
   * @param modules The set of modules to which the cell belongs.
   * @param name The name of the cell.
   * @param type The type of the cell.
   * @param ports The vector of ports associated with the cell.
   * @param hidden A flag indicating whether the cell is hidden.
   * @param parameters The JSON object containing the parameters of the cell.
   * @param attributes The JSON object containing the attributes of the cell.
   */
  void add_cell(const std::unordered_set<std::string>& modules, const std::string& name, const std::string& type, std::vector<ducode::Port>& ports, bool hidden, const nlohmann::json& parameters, const nlohmann::json& attributes);

  /**
   * @brief Returns the name of the module.
   * 
   * @return The name of the module as a string.
   */
  [[nodiscard]] std::string get_name() const {
    return m_name;
  }

  /**
   * Sets the source information for the module.
   *
   * @param src The name of the source file.
   * @param line The line number in the source file.
   */
  void set_source_info(const std::string& src, uint32_t line) {
    m_src_name = src;
    m_src_line = line;
  }

  /**
   * Returns the source information of the module.
   *
   * The source information is a string representation of the source file name and line number
   * where the module is defined.
   *
   * @return A string containing the source information in the format "filename:line_number".
   */
  [[nodiscard]] std::string get_source_info() const {
    return fmt::format("{}:{}", m_src_name, m_src_line);
  }

  /**
   * @brief Returns a const reference to the vector of nets.
   *
   * This function returns a const reference to the vector of nets in the module.
   * The returned vector cannot be modified.
   *
   * @return A const reference to the vector of nets.
   */
  [[nodiscard]] const std::vector<Net>& get_nets() const {
    return m_nets;
  }

  /**
   * @brief Retrieves the names of all input ports in the module.
   * 
   * @return A vector of strings containing the names of the input ports.
   */
  [[nodiscard]] std::vector<std::string> get_input_names() const {
    std::vector<std::string> names;
    for (const auto& port: m_ports) {
      if (port.m_direction == Port::Direction::input) {
        names.emplace_back(port.m_name);
      }
    }
    return names;
  }

  /**
   * @brief Retrieves the names of the output ports.
   * 
   * This function returns a vector of strings containing the names of the output ports
   * in the module.
   * 
   * @return A vector of strings representing the names of the output ports.
   */
  [[nodiscard]] std::vector<std::string> get_output_names() const {
    std::vector<std::string> names;
    for (const auto& port: m_ports) {
      if (port.m_direction == Port::Direction::output) {
        names.emplace_back(port.m_name);
      }
    }
    return names;
  }

  /**
   * @brief Retrieves the names of the input/output ports in the module.
   * 
   * This function returns a vector of strings containing the names of the input/output ports
   * in the module that have a bidirectional direction (inout).
   * 
   * @return A vector of strings containing the names of the inout ports.
   */
  [[nodiscard]] std::vector<std::string> get_inout_names() const {
    std::vector<std::string> names;
    for (const auto& port: m_ports) {
      if (port.m_direction == Port::Direction::inout) {
        names.emplace_back(port.m_name);
      }
    }
    return names;
  }

  /**
   * @brief Adds a Net to the module.
   *
   * This function adds a Net to the module's collection of nets. If a net with the same bits signature already exists in the collection, it will be replaced under certain conditions. If the existing net is hidden, the new net will replace it regardless of its hidden status. If the existing net is not hidden and the new net is also not hidden, the new net will be added to the collection.
   *
   * @param net The Net to be added.
   */
  void add_net(const Net& net) {
    auto net_it = std::ranges::find_if(m_nets, [&](const Net& local_net) { return local_net.get_bits_signature() == net.get_bits_signature(); });
    if (net_it == m_nets.end()) {
      m_nets.emplace_back(net);
      return;
    }
    if (net_it->is_hidden()) {
      *net_it = net;
    } else if (!net_it->is_hidden() && !net.is_hidden()) {
      m_nets.emplace_back(net);
    }
  }

  /**
   * Populates the module graph by adding vertices and edges based on the nets and cells in the module.
   * 
   * This function iterates over the nets and cells in the module and adds corresponding vertices to the module graph.
   * It also connects the vertices based on the matching nets and ports.
   * 
   * @throws std::runtime_error if an unsupported case is encountered.
   */
  void populate_graph() {
    auto vectors_intersection = [](const fast_bit_container& bits1, const fast_bit_container& bits2) -> bool {
      if (bits1.empty() || bits2.empty()) {
        std::runtime_error("Empty bits!");
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

        auto net_range = subrange(net_signatures.equal_range(net.get_bits_signature()));
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

  /**
   * @brief Returns the module graph.
   * 
   * This function returns a reference to the module graph.
   * 
   * @return const ModuleGraph& A reference to the module graph.
   */
  [[nodiscard]] const ModuleGraph& get_graph() const {
    return m_module_graph;
  }

  /**
   * Writes the module graph in Graphviz format to the specified file.
   * 
   * @param filename The name of the file to write the Graphviz output to.
   */
  void write_graphviz(const std::string& filename) const {
    auto file = std::ofstream(filename, std::ofstream::out);
    boost::write_graphviz(file, m_module_graph,
                          boost::make_label_writer(boost::get(&VertexInfo::label, m_module_graph)),
                          boost::make_label_writer(boost::get(&EdgeInfo::label, m_module_graph)));
  }

  /**
   * @brief Exports the Verilog header for the module.
   *
   * This function generates the Verilog module header based on the provided parameters.
   * It includes the module name and the list of input, output, and inout ports.
   * If the 'ift' parameter is set to true, it also includes additional typedefs for each port.
   *
   * @param params The JSON object containing the parameters for generating the Verilog header.
   *               It should contain the 'ift' parameter indicating whether to include typedefs.
   * @return A string representing the Verilog module header.
   *
   * @throws std::runtime_error if the port direction is not recognized.
   */
  [[nodiscard]] std::string export_verilog_header(const nlohmann::json& params) const {
    Expects(params.contains("ift"));

    const bool ift = params["ift"].get<bool>();

    auto input_names = get_input_names();
    auto output_names = get_output_names();
    auto inout_names = get_inout_names();

    if (ift) {
      input_names.clear();
      for (const auto& name: get_input_names()) {
        input_names.emplace_back(legalize_identifier(name));
        input_names.emplace_back(fmt::format("{}_t", legalize_identifier(name)));
      }
      output_names.clear();
      for (const auto& name: get_output_names()) {
        output_names.emplace_back(legalize_identifier(name));
        output_names.emplace_back(fmt::format("{}_t", legalize_identifier(name)));
      }
      inout_names.clear();
      for (const auto& name: get_inout_names()) {
        inout_names.emplace_back(legalize_identifier(name));
        inout_names.emplace_back(fmt::format("{}_t", legalize_identifier(name)));
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
        result << fmt::format("{} [{}:0] {}_t;\n", direction_string, tag_size - 1, legalize_identifier(port.m_name));
      }
    }

    return result.str();
  }

  /**
   * @brief Exports the module to Verilog code.
   * 
   * This function exports the module to Verilog code based on the provided parameters.
   * It iterates over the module graph and generates Verilog code for each vertex in the graph.
   * The generated Verilog code includes module headers, cell exports, net exports, and port exports.
   * 
   * @param params The parameters for exporting the module.
   * @return A string containing the Verilog code representation of the module.
   */
  [[nodiscard]] std::string export_verilog(const nlohmann::json& params) const {
    bool ift = false;
    if (params.contains("ift")) {
      ift = params["ift"].get<bool>();
    }

    std::stringstream result;

    result << export_verilog_header(params);

    for (auto vertex: subrange(boost::vertices(m_module_graph))) {
      std::vector<ModuleGraph::vertex_descriptor> sources;
      std::string target_name;
      bit_container target_bits;

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
      } else if (m_module_graph[vertex].type == VertexType::kNet) {
        Expects(m_module_graph[vertex].net_ptr != nullptr);
        const auto* net_ptr = m_module_graph[vertex].net_ptr;
        target_name = legalize_identifier(net_ptr->get_name());
        target_bits = net_ptr->get_bits();
        auto net_params = params;
        auto net_sources = subrange(boost::inv_adjacent_vertices(vertex, m_module_graph));
        for (const auto& source: net_sources) {
          sources.emplace_back(source);
          if (m_module_graph[source].type == VertexType::kCell && m_module_graph[source].cell_ptr->has_register()) {
            net_params["is_register"] = true;
          }
        }
        result << net_ptr->export_verilog(net_params);
      } else if (m_module_graph[vertex].type == VertexType::kCellPort) {
        Expects(m_module_graph[vertex].port_ptr != nullptr && m_module_graph[vertex].cell_ptr != nullptr && m_module_graph[vertex].net_ptr == nullptr);
        const auto* port_ptr = m_module_graph[vertex].port_ptr;
        target_name = legalize_identifier(port_ptr->m_identifier);
        target_bits = port_ptr->m_bits;
        auto port_params = params;
        auto port_sources = subrange(boost::inv_adjacent_vertices(vertex, m_module_graph));
        for (const auto& source: port_sources) {
          sources.emplace_back(source);
          if (m_module_graph[source].type == VertexType::kCell && m_module_graph[source].cell_ptr->has_register()) {
            port_params["is_register"] = true;
          }
        }
        result << port_ptr->export_verilog(port_params);
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
          ift_source_names.emplace_back(source_name + "_t");
        }
      }
      if (!ift_source_names.empty()) {
        result << fmt::format(" assign {}_t = {};\n", target_name, boost::join(ift_source_names, " | "));
      }
    }

    result << "endmodule" << '\n';

    return result.str();
  }
};

}// namespace ducode

#include "detail/module_inl.hpp"