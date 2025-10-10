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
class CellMux : public Cell {
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
      : Cell(std::move(name), std::move(type), ports, hidden, parameters, attributes) {
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
      // result << ift_assignment(y, {a, b, s});
      static constexpr std::string_view str = "assign {}{} = (^{} === 1'bx ? 0 : ({} ? {}{} : {}{})) | (^({}^{}) === 1'bx ? 0 : ( (^({} ^ {}) === 1'b1) ? {}{} : 0)) | ({}{} & {}{}) | ({}{} & {}{});\n";
      result << fmt::format(str, y, ift_tag, s, s, b, ift_tag, a, ift_tag, a, b, a, b, s, ift_tag, a, ift_tag, s, ift_tag, b, ift_tag, s, ift_tag);
    }

    return result.str();
  }

  void export_smt2(z3::context& ctx, z3::solver& solver, const std::unordered_map<std::string, z3::expr>& port_expr_map, [[maybe_unused]] const nlohmann::json& params) const override {
    assert(port_expr_map.size() <= 4);

    solver.add((port_expr_map.at("Y") == to_expr(ctx, Z3_mk_ite(ctx, (port_expr_map.at("S") == 1), port_expr_map.at("B"), port_expr_map.at("A")))));
  }
};
}// namespace ducode