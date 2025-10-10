/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/cell/basic_gate.hpp>
#include <ducode/utility/legalize_identifier.hpp>

namespace ducode {

/** \brief \c CellShiftx represents a shift register with two predecessor nets;
* pred_0 = dataIn (A), pred_1 = shiftDirection (B) and the successor data output Y;
*/

class CellShiftx : public Cell {
  std::size_t Y_width;
  bool A_signed;
  bool B_signed;

public:
  CellShiftx(std::string name, std::string type, std::vector<ducode::Port>& ports, bool hidden, const nlohmann::json& parameters, const nlohmann::json& attributes)
      : Cell(std::move(name), std::move(type), ports, hidden, parameters, attributes) {
    // One output
    assert(ports.size() == 3);
    assert(ports[0].m_name == "A");
    assert(ports[0].m_direction == ducode::Port::Direction::input);
    assert(ports[1].m_name == "B");
    assert(ports[1].m_direction == ducode::Port::Direction::input);
    assert(ports[2].m_name == "Y");
    assert(ports[2].m_direction == ducode::Port::Direction::output);
    Y_width = ports[2].m_bits.size();
    A_signed = ports[0].m_signed;
    B_signed = ports[1].m_signed;
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
    std::string a;
    std::string b;
    std::string y;

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
    }

    if (B_signed) {
      result << fmt::format("  assign {} = {}[$signed({}) +: {}];\n", y, a, b, Y_width);
    } else {
      result << fmt::format("  assign {} = {}[{} +: {}];\n", y, a, b, Y_width);
    }

    if (ift) {
      result << fmt::format("  assign {} = {} | {};\n", y + ift_tag, a + ift_tag, b + ift_tag);
    }

    return result.str();
  }

  void export_smt2(z3::context& ctx, z3::solver& solver, const std::unordered_map<std::string, z3::expr>& port_expr_map, [[maybe_unused]] const nlohmann::json& params) const override {

    assert(port_expr_map.size() <= 3);

    unsigned largest_bit_size = (port_expr_map.at("A").get_sort().bv_size() < port_expr_map.at("B").get_sort().bv_size()) ? port_expr_map.at("B").get_sort().bv_size() : port_expr_map.at("A").get_sort().bv_size();
    z3::expr extended_a = A_signed ? z3::sext(port_expr_map.at("A"), (largest_bit_size - port_expr_map.at("A").get_sort().bv_size())) : z3::zext(port_expr_map.at("A"), (largest_bit_size - port_expr_map.at("A").get_sort().bv_size()));
    z3::expr extended_b = B_signed ? z3::sext(port_expr_map.at("B"), (largest_bit_size - port_expr_map.at("B").get_sort().bv_size())) : z3::zext(port_expr_map.at("B"), (largest_bit_size - port_expr_map.at("B").get_sort().bv_size()));
    z3::expr extended_b_inv = to_expr(ctx, Z3_mk_bvadd(ctx, to_expr(ctx, Z3_mk_bvnot(ctx, extended_b)), ctx.bv_val(1, largest_bit_size)));
    if (B_signed) {
      if (A_signed) {
        solver.add(z3::sext(port_expr_map.at("Y"), (largest_bit_size - port_expr_map.at("Y").get_sort().bv_size())) == to_expr(ctx, Z3_mk_ite(ctx, (extended_b.extract(extended_b.get_sort().bv_size() - 1, extended_b.get_sort().bv_size() - 1) == ctx.bv_val(1, 1)), to_expr(ctx, Z3_mk_bvshl(ctx, extended_a, extended_b_inv)), to_expr(ctx, Z3_mk_bvlshr(ctx, extended_a, extended_b)))));
      } else {
        solver.add(z3::zext(port_expr_map.at("Y"), (largest_bit_size - port_expr_map.at("Y").get_sort().bv_size())) == to_expr(ctx, Z3_mk_ite(ctx, (extended_b.extract(extended_b.get_sort().bv_size() - 1, extended_b.get_sort().bv_size() - 1) == ctx.bv_val(1, 1)), to_expr(ctx, Z3_mk_bvshl(ctx, extended_a, extended_b_inv)), to_expr(ctx, Z3_mk_bvlshr(ctx, extended_a, extended_b)))));
      }
    } else {
      solver.add(z3::zext(port_expr_map.at("Y"), (largest_bit_size - port_expr_map.at("Y").get_sort().bv_size())) == to_expr(ctx, Z3_mk_bvlshr(ctx, extended_a, extended_b)));
    }
  }
};
}// namespace ducode