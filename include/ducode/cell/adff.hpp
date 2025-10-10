/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/cell/basic_dff.hpp>
#include <ducode/port.hpp>

#include <fmt/format.h>
#include <gsl/assert>
#include <gsl/narrow>
#include <nlohmann/json.hpp>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ducode {

/** \brief \c CellADFF represents a DFF register of a certain width with four predecessor nets;
pred_0= ARST, pred_1= CLK, pred_2 = dataIn, the succssor is the data output.
It has a constant asynchronous reset value triggerd with the arst input
*/

class CellADFF : public CellBasicDFF {
  bool arst_active_high{};
  std::string arst_value;

public:
  CellADFF(std::string name, std::string type, std::vector<ducode::Port>& ports, bool hidden, const nlohmann::json& parameters, const nlohmann::json& attributes)
      : CellBasicDFF(std::move(name), std::move(type), ports, hidden, parameters, attributes) {
    // One output
    assert(ports.size() == 4);
    assert(ports[0].m_name == "ARST");
    assert(ports[0].m_direction == ducode::Port::Direction::input);
    assert(ports[1].m_name == "CLK");
    assert(ports[1].m_direction == ducode::Port::Direction::input);
    assert(ports[2].m_name == "D");
    assert(ports[2].m_direction == ducode::Port::Direction::input);
    assert(ports[3].m_name == "Q");
    assert(ports[3].m_direction == ducode::Port::Direction::output);
    assert(ports[0].m_bits.size() == ports[1].m_bits.size());
    assert(ports[2].m_bits.size() == ports[3].m_bits.size());
    // Clock is the second input

    clk_pos_edge = std::stoull(parameters.at("CLK_POLARITY").get<std::string>(), nullptr, 2) != 0;
    arst_active_high = std::stoull(parameters.at("ARST_POLARITY").get<std::string>(), nullptr, 2) != 0;
    arst_value = parameters.at("ARST_VALUE").get<std::string>();
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
    std::string arst;

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
      if (port.m_name == "ARST") {
        Ensures(port.m_direction == ducode::Port::Direction::input);
        arst = identifiers_map.at(port.m_bits_signature);
      }
    }

    std::string clk_edge = clk_pos_edge ? "posedge" : "negedge";
    std::string arst_edge = arst_active_high ? "posedge" : "negedge";
    std::string neg = arst_active_high ? "" : "!";
    result << fmt::format("  always @({} {}, {} {}) begin\n", clk_edge, clk, arst_edge, arst)
           << fmt::format("      if ({}{}) begin\n", neg, arst)
           << fmt::format("          {} <= {}'b{};\n", q, width, arst_value);
    if (ift) {
      result << fmt::format("          {}{} <= (^{} === 1'bx) ? 0 : (({} == {}'b{}) ? ({}{} | {}{}) : {}{});\n", q, ift_tag, d, d, width, arst_value, d, ift_tag, arst, ift_tag, arst, ift_tag);
    }
    result << "      end else begin\n";
    result << fmt::format("          {} <= {};\n", q, d);
    if (ift) {
      result << fmt::format("          {}{} <= (^{} === 1'bx) ? 0 : (({} == {}'b{}) ? ({}{} | {}{}) : {}{});\n", q, ift_tag, d, d, width, arst_value, d, ift_tag, arst, ift_tag, d, ift_tag);
    }
    result << "       end\n";
    result << "  end\n\n";

    return result.str();
  }

  void export_smt2(z3::context& ctx, z3::solver& solver, const std::unordered_map<std::string, z3::expr>& port_expr_map, [[maybe_unused]] const nlohmann::json& params) const override {
    // throw std::runtime_error("ADFF cell Not validated for smt2-simulation!! Would only work with specific configurations treating it like a synchronous element.");

    assert(port_expr_map.size() <= 4);

    z3::expr arst_val(ctx);
    if (arst_value.contains("x") || arst_value.contains("z")) {
      arst_val = ctx.bv_const(arst_value.c_str(), gsl::narrow<uint32_t>(width));
    } else {
      uint64_t arst_value_dec = std::stoul(arst_value, nullptr, 2);
      arst_val = ctx.bv_val(arst_value_dec, gsl::narrow<uint32_t>(width));
    }

    if (arst_active_high) {// stoi (arst_value) when arst_value is a string of "x" or "z" create a bv_const(x, bv_size) instead
      solver.add((port_expr_map.at("Q") == to_expr(ctx, Z3_mk_ite(ctx, (port_expr_map.at("ARST") == 1), arst_val, port_expr_map.at("D")))));
    } else {
      solver.add((port_expr_map.at("Q") == to_expr(ctx, Z3_mk_ite(ctx, (port_expr_map.at("ARST") == 0), arst_val, port_expr_map.at("D")))));
    }
  }
};

}// namespace ducode