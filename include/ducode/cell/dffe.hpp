/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/cell/basic_dff.hpp>

#include <utility>

namespace ducode {

/** \brief \c CellADFF represents a DFF register of a certain width with three predecessor nets;
    pred_0 = RST, pred_1 = CLK, pred_2= dataIn, the succssor is the data output.
*/

class CellDFFE : public CellBasicDFF {
  bool en_active_high;

public:
  CellDFFE(std::string name, std::string type, std::vector<ducode::Port>& ports, bool hidden, const nlohmann::json& parameters, const nlohmann::json& attributes)
      : CellBasicDFF(std::move(name), std::move(type), ports, hidden, parameters, attributes) {
    // One output
    assert(ports.size() == 4);
    assert(ports[0].m_name == "CLK");
    assert(ports[0].m_direction == ducode::Port::Direction::input);
    assert(ports[1].m_name == "D");
    assert(ports[1].m_direction == ducode::Port::Direction::input);
    assert(ports[2].m_name == "EN");
    assert(ports[2].m_direction == ducode::Port::Direction::input);
    assert(ports[3].m_name == "Q");
    assert(ports[3].m_direction == ducode::Port::Direction::output);
    assert(ports[0].m_bits.size() == 1);
    assert(ports[2].m_bits.size() == 1);
    assert(ports[1].m_bits.size() == ports[3].m_bits.size());
    // Clock is the first input
    en_active_high = std::stoull(parameters.at("EN_POLARITY").get<std::string>(), nullptr, 2) != 0;
  }

  [[nodiscard]] std::string export_verilog(const nlohmann::json& params) const override {
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
    std::string en;

    for (const auto& port: m_ports) {
      if (port.m_direction == ducode::Port::Direction::output) {
        assert(port.m_name == "Q");
        q = identifiers_map.at(port.m_bits_signature);
      }
      if (port.m_name == "CLK") {
        Ensures(port.m_direction == ducode::Port::Direction::input);
        clk = identifiers_map.at(port.m_bits_signature);
      }
      if (port.m_name == "D") {
        Ensures(port.m_direction == ducode::Port::Direction::input);
        d = identifiers_map.at(port.m_bits_signature);
      }
      if (port.m_name == "EN") {
        Ensures(port.m_direction == ducode::Port::Direction::input);
        en = identifiers_map.at(port.m_bits_signature);
      }
    }

    std::string clk_edge;
    std::string en_polarity;
    clk_edge = clk_pos_edge ? "posedge" : "negedge";
    en_polarity = en_active_high ? "" : "!";
    result << fmt::format("  always @({} {}) begin\n", clk_edge, clk);
    result << fmt::format("      if({}{}) begin\n", en_polarity, en);
    result << fmt::format("           {} <= {};\n", q, d);
    if (ift) {
      result << fmt::format("           {}{} <= {}{} | {}{};\n", q, ift_tag, d, ift_tag, en, ift_tag);
    }
    result << "      end\n";
    result << "  end\n\n";

    return result.str();
  }

  void export_smt2(z3::context& ctx, z3::solver& solver, const std::unordered_map<std::string, z3::expr>& port_expr_map, [[maybe_unused]] const nlohmann::json& params) const override {

    assert(port_expr_map.size() <= 5);// 4 ports: CLK, D, EN, Q and 1 port from the previous timestep: Q_prev

    if (en_active_high) {
      solver.add((port_expr_map.at("Q") == to_expr(ctx, Z3_mk_ite(ctx, (port_expr_map.at("EN") == 1), port_expr_map.at("D"), port_expr_map.at("Q_prev")))));
    } else {
      solver.add((port_expr_map.at("Q") == to_expr(ctx, Z3_mk_ite(ctx, (port_expr_map.at("EN") == 0), port_expr_map.at("D"), port_expr_map.at("Q_prev")))));
    }
  }
};
}// namespace ducode