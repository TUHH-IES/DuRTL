/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/cell.hpp>
#include <ducode/detail/module_graph.hpp>
#include <ducode/net.hpp>
#include <ducode/port.hpp>
#include <ducode/types.hpp>
#include <ducode/utility/legalize_identifier.hpp>

#include <boost/graph/detail/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/poly_collection/base_collection.hpp>
#include <fmt/core.h>
#include <nlohmann/json_fwd.hpp>
#include <spdlog/spdlog.h>

#include <algorithm>
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
  std::vector<Net> m_nets;
  boost::base_collection<Cell> m_cells;
  std::vector<Port> m_ports;
  ModuleGraph m_module_graph;
  nlohmann::json m_parameters;
  nlohmann::json m_attributes;

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

  [[nodiscard]] bool check_clock_signal_consistency() const;
  [[nodiscard]] bool check_clock_rising_edge_consistency() const;
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
   * @brief Retrieves references to all input ports in the module.
   * 
   * This function returns a vector of reference wrappers to all the input ports
   * in the module. Using reference wrappers allows storing references in a 
   * standard container while maintaining access to the original port objects.
   * 
   * @return A vector of reference wrappers to the input ports.
   */
  [[nodiscard]] std::vector<std::reference_wrapper<const Port>> get_input_ports() const {
    std::vector<std::reference_wrapper<const Port>> port_refs;
    port_refs.reserve(m_ports.size());
    for (const auto& port: m_ports) {
      if (port.m_direction == Port::Direction::input) {
        port_refs.emplace_back(port);
      }
    }
    return port_refs;
  }

  /**
   * @brief Retrieves references to all input ports in the module.
   * 
   * This function returns a vector of reference wrappers to all the input ports
   * in the module. Using reference wrappers allows storing references in a 
   * standard container while maintaining access to the original port objects.
   * 
   * @return A vector of reference wrappers to the input ports.
   */
  [[nodiscard]] std::vector<std::reference_wrapper<const Port>> get_output_ports() const {
    std::vector<std::reference_wrapper<const Port>> port_refs;
    port_refs.reserve(m_ports.size());
    for (const auto& port: m_ports) {
      if (port.m_direction == Port::Direction::output) {
        port_refs.emplace_back(port);
      }
    }
    return port_refs;
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
  void populate_graph();

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
  [[nodiscard]] std::string export_verilog_header(const nlohmann::json& params) const;

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
  [[nodiscard]] std::string export_verilog(const nlohmann::json& params) const;

  nlohmann::json& get_attributes() { return m_attributes; }
};

}// namespace ducode
