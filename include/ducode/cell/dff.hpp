/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/cell/basic_dff.hpp>

#include <utility>

namespace ducode {

/** \brief \c CellDFF represents a DFF register of a certain width with two predecessor nets;
pred_0= CLK, pred_1= dataIn, the succssor is the data output Q.
*/

class CellDFF : public CellBasicDFF {

public:
  CellDFF(std::string name, std::string type, std::vector<ducode::Port>& ports, bool hidden, const nlohmann::json& parameters, const nlohmann::json& attributes)
      : CellBasicDFF(std::move(name), std::move(type), ports, hidden, parameters, attributes) {
    // One output
    assert(ports.size() == 3);
    assert(ports[0].m_name == "CLK");
    assert(ports[0].m_direction == ducode::Port::Direction::input);
    assert(ports[1].m_name == "D");
    assert(ports[1].m_direction == ducode::Port::Direction::input);
    assert(ports[2].m_name == "Q");
    assert(ports[2].m_direction == ducode::Port::Direction::output);
    assert(ports[0].m_bits.size() == 1);
    assert(ports[1].m_bits.size() == ports[2].m_bits.size());
    // Clock is the first input
  }

  [[nodiscard]] std::string export_verilog(const nlohmann::json& params) const override {
    assert(m_ports.size() == 3);

    assert(params.contains("ift"));

    const bool ift = params["ift"].get<bool>();

    std::unordered_map<std::size_t, std::string> identifiers_map;
    if (params.contains("identifiers_map")) {
      identifiers_map = params["identifiers_map"].get<std::unordered_map<std::size_t, std::string>>();
    }

    std::stringstream result;
    std::string clk;
    std::string d;
    std::string q;

    //assigning the ports
    for (const auto& port: m_ports) {
      if (port.m_direction == ducode::Port::Direction::output) {
        Ensures(port.m_name == "Q");
        q = identifiers_map.at(port.m_bits_signature);
      }
      if (port.m_name == "D") {
        Ensures(port.m_direction == ducode::Port::Direction::input);
        d = identifiers_map.at(port.m_bits_signature);
      }
      if (port.m_name == "CLK") {
        Ensures(port.m_direction == ducode::Port::Direction::input);
        clk = identifiers_map.at(port.m_bits_signature);
      }
    }

    std::string clk_edge;
    clk_edge = clk_pos_edge ? "posedge" : "negedge";

    //output of the flipflop
    result << fmt::format("  always @({} {}) begin\n", clk_edge, clk);
    result << fmt::format("      {} <= {};\n", q, d);

    if (ift) {
      result << fmt::format("      {}{} <= {}{};\n", q, ift_tag, d, ift_tag);
    }
    result << "  end\n\n";

    return result.str();
  }

  /// CellBasicGate calls a general export function for basicgates which have a common verilog syntax and mostly different because of the used operator
  void export_smt2([[maybe_unused]] z3::context& ctx, z3::solver& solver, const std::unordered_map<std::string, z3::expr>& port_expr_map, [[maybe_unused]] const nlohmann::json& params) const override {

    assert(port_expr_map.size() <= 3);

    solver.add(port_expr_map.at("Q") == port_expr_map.at("D"), fmt::format("edge_{}=={}", port_expr_map.at("Q").to_string(), port_expr_map.at("D").to_string()).c_str());
  }
};
}// namespace ducode