/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/cell/basic_gate.hpp>

#include <string>

namespace ducode {

/**
 * @brief Represents a multiplexer cell.
 *
 * The `CellMux` class is a derived class of `CellBasicGate` and represents a multiplexer cell.
 * It provides functionality to export the Verilog representation of the cell based on the provided parameters.
 */
class CellMux : public CellBasicGate {
public:
  /**
   * @brief Constructs a CellMux object.
   *
   * This constructor initializes a CellMux object with the specified parameters.
   *
   * @param name The name of the CellMux.
   * @param type The type of the CellMux.
   * @param ports The vector of ports associated with the CellMux.
   * @param hidden A boolean value indicating whether the CellMux is hidden.
   * @param parameters The JSON object containing the parameters of the CellMux.
   * @param attributes The JSON object containing the attributes of the CellMux.
   */
  CellMux(std::string name, std::string type, std::vector<ducode::Port>& ports, bool hidden, const nlohmann::json& parameters, const nlohmann::json& attributes)
      : CellBasicGate(std::move(name), std::move(type), ports, hidden, parameters, attributes) {
  }

  /**
   * @brief Returns the Verilog operator for the cell.
   *
   * This function returns the Verilog operator that corresponds to the cell's operation.
   *
   * @return The Verilog operator as a string.
   */
  [[nodiscard]] std::string get_verilog_operator() const override {
    return " ";
  }

  /**
   * @brief Exports the Verilog representation of the cell.
   *
   * This function exports the Verilog representation of the cell based on the provided parameters.
   *
   * @param params The parameters for exporting the Verilog representation.
   * @return The Verilog representation of the cell as a string.
   */
  [[nodiscard]] std::string export_verilog(const nlohmann::json& params) const override {
    assert(m_ports.size() == 4);

    assert(params.contains("ift"));

    const bool ift = params["ift"].get<bool>();

    std::unordered_map<std::size_t, std::string> identifiers_map;
    if (params.contains("identifiers_map")) {
      identifiers_map = params["identifiers_map"].get<std::unordered_map<std::size_t, std::string>>();
    }

    std::stringstream result;

    // strings for the ports
    std::string a;
    std::string b;
    std::string y;
    std::string s;

    for (const auto& port: m_ports) {
      if (port.m_direction == ducode::Port::Direction::output) {
        assert(port.m_name == "Y");
        y = identifiers_map.at(port.m_bits_signature);
      }
      if (port.m_name == "A") {
        Ensures(port.m_direction == ducode::Port::Direction::input);
        a = identifiers_map.at(port.m_bits_signature);
      }
      if (port.m_name == "B") {
        Ensures(port.m_direction == ducode::Port::Direction::input);
        b = identifiers_map.at(port.m_bits_signature);
      }
      if (port.m_name == "S") {
        Ensures(port.m_direction == ducode::Port::Direction::input);
        s = identifiers_map.at(port.m_bits_signature);
      }
    }

    result << fmt::format("  assign {} = {} ? {} : {};\n", y, s, b, a);
    if (ift) {
      result << ift_assignment(y, {a, b, s});
    }

    return result.str();
  }

  /**
   * Calculates the if-then-else assignment for a given successor and predecessors.
   *
   * @param successor The successor variable.
   * @param predecessors The vector of predecessor variables.
   * @return The if-then-else assignment as a string.
   */
  [[nodiscard]] std::string ift_assignment(const std::string& successor, const std::vector<std::string>& predecessors) const override {
    assert(!successor.empty());
    assert(predecessors.size() == 3);

    static constexpr std::string_view s = "assign {}{} = (^{} === 1'bx ? 0 : ({} ? {}{} : {}{})) | (^({}^{}) === 1'bx ? 0 : ( (^({} ^ {}) === 1'b1) ? {}{} : 0)) | ({}{} & {}{}) | ({}{} & {}{});\n";
    return fmt::format(s, successor, ift_tag, predecessors[2], predecessors[2], predecessors[1], ift_tag, predecessors[0], ift_tag, predecessors[0], predecessors[1], predecessors[0], predecessors[1], predecessors[2], ift_tag, predecessors[0], ift_tag, predecessors[2], ift_tag, predecessors[1], ift_tag, predecessors[2], ift_tag);

    //      result << fmt::format("  assign {}{} = {} ? {}{} : {}{} ;\n", y, ift_tag, predecessors[2], predecessors[1], ift_tag, predecessors[0], ift_tag);
    //      result << fmt::format("  assign {}{} = (({}{}>0) && ({} ^ {})) ? ({}{} | {}{} | {}{}) : ({} ? {}{} : {}{}) ;\n", y, ift_tag, predecessors[2], ift_tag, predecessors[0], predecessors[1], predecessors[1], ift_tag, predecessors[0], ift_tag, predecessors[2], ift_tag, predecessors[2], predecessors[1], ift_tag, predecessors[0], ift_tag);
  }
};
}// namespace ducode