/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/cell.hpp>

#include <utility>

namespace ducode {

class CellBasicGate : public Cell {

public:
  CellBasicGate(std::string name, std::string type, std::vector<ducode::Port>& ports, bool hidden, const nlohmann::json& parameters, const nlohmann::json& attributes)
      : Cell(std::move(name), std::move(type), ports, hidden, parameters, attributes) {
    assert(std::ranges::count_if(ports, [](const auto& port) { return port.m_direction == ducode::Port::Direction::output; }) == 1);
  }

  [[nodiscard]] virtual std::string get_verilog_operator() const = 0;

  /// CellBasicGate calls a general export function for basicgates which have a common verilog syntax and mostly different because of the used operator
  [[nodiscard]] std::string export_verilog(const nlohmann::json& params) const override {
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

    if (m_ports.size() == 2) {
      result << fmt::format("  assign {} = {}{};\n", y, get_verilog_operator(), a);
      if (ift) {
        result << ift_assignment(y, {a});
      }
    } else if (m_ports.size() == 3) {
      result << fmt::format("  assign {} = {} {} {};\n", y, a, get_verilog_operator(), b);
      if (ift) {
        result << ift_assignment(y, {a, b});
      }
    } else {
      throw std::runtime_error("more than 3 ports in a basic gate");
    }

    return result.str();
  }

  /// CellBasicGate calls a general export function for basicgates which have a common verilog syntax and mostly different because of the used operator
  void export_smt2(z3::context& ctx, z3::solver& solver, const std::unordered_map<std::string, z3::expr>& port_expr_map, [[maybe_unused]] const nlohmann::json& params) const override {
    assert(port_expr_map.size() <= 3);

    if (m_ports.size() == 2) {
      solver.add(get_cell_assertion_unary(ctx, port_expr_map.at("A"), port_expr_map.at("Y")));
    } else if (m_ports.size() == 3) {
      solver.add(get_cell_assertion_binary(ctx, port_expr_map.at("A"), port_expr_map.at("B"), port_expr_map.at("Y")));
    } else {
      throw std::runtime_error("more than 3 ports in a basic gate");
    }
  }

  [[nodiscard]] virtual std::string ift_assignment([[maybe_unused]] const std::string& successor, [[maybe_unused]] const std::vector<std::string>& predecessors) const = 0;
};

class CellBasicGateUnary : public CellBasicGate {
public:
  CellBasicGateUnary(std::string name, std::string type, std::vector<ducode::Port>& ports, bool hidden, const nlohmann::json& parameters, const nlohmann::json& attributes)
      : CellBasicGate(std::move(name), std::move(type), ports, hidden, parameters, attributes) {
  }

  [[nodiscard]] std::string get_verilog_operator() const override {
    if (m_type == "$logic_not") {
      return "!";
    }
    if (m_type == "$not") {
      return "~";
    }
    if (m_type == "$pos") {
      return "";
    }
    if (m_type == "$neg") {
      return "-";
    }

    throw std::runtime_error("Unknown type.");
  }

  [[nodiscard]] std::string ift_assignment(const std::string& successor, const std::vector<std::string>& predecessors) const override {
    assert(!successor.empty());
    assert(predecessors.size() == 1);

    static constexpr std::string_view s = "assign {}{} = {}{} ;\n";
    return fmt::format(s, successor, ift_tag, predecessors[0], ift_tag);
  }

  [[nodiscard]] z3::expr get_cell_assertion_unary(z3::context& ctx, const z3::expr& input1, const z3::expr& output) const override {
    (void) ctx;
    if (m_type == "$logic_not") {
      return (output == to_expr(ctx, Z3_mk_ite(ctx, (input1 == 0), ctx.bv_val(1, 1), ctx.bv_val(0, 1))));
    }
    if (m_type == "$not") {
      return (output == ~input1);
    }
    if (m_type == "$pos") {
      if (input1.get_sort().bv_size() < output.get_sort().bv_size()) {
        return (output == z3::zext(input1, output.get_sort().bv_size() - input1.get_sort().bv_size()));
      }
      return (output == input1);
    }
    if (m_type == "$neg") {
      // throw std::runtime_error("possibly not implemented correctly!!");
      return (output == ~input1 + 1);
    }

    throw std::runtime_error("Unknown type.");
  }
};

class CellBasicGateBinary : public CellBasicGate {
public:
  CellBasicGateBinary(std::string name, std::string type, std::vector<ducode::Port>& ports, bool hidden, const nlohmann::json& parameters, const nlohmann::json& attributes)
      : CellBasicGate(std::move(name), std::move(type), ports, hidden, parameters, attributes) {
  }

  [[nodiscard]] std::string get_verilog_operator() const override {
    if (m_type == "$add") {
      return "+";
    }
    if (m_type == "$sub") {
      return "-";
    }
    if (m_type == "$mul") {
      return "*";
    }
    if (m_type == "$div") {
      return "/";
    }
    if (m_type == "$eq") {
      return "==";
    }
    if (m_type == "$eqx") {
      return "===";
    }
    if (m_type == "$ge") {
      return ">=";
    }
    if (m_type == "$gt") {
      return ">";
    }
    if (m_type == "$le") {
      return "<=";
    }
    if (m_type == "$lt") {
      return "<";
    }
    if (m_type == "$mod") {
      return "%";
    }
    if (m_type == "$ne") {
      return "!=";
    }
    if (m_type == "$nex") {
      return "!==";
    }
    if (m_type == "$pow") {
      return "**";
    }
    if (m_type == "$shl") {
      return "<<";
    }
    if (m_type == "$shr") {
      return ">>";
    }
    if (m_type == "$sshl") {
      return "<<<";
    }
    if (m_type == "$sshr") {
      return ">>>";
    }

    throw std::runtime_error("Unknown type.");
  }

  [[nodiscard]] std::string ift_assignment(const std::string& successor, const std::vector<std::string>& predecessors) const override {
    assert(!successor.empty());
    assert(predecessors.size() == 2);

    static constexpr std::string_view s = "assign {}{} = {}{} | {}{} ;\n";
    return fmt::format(s, successor, ift_tag, predecessors[0], ift_tag, predecessors[1], ift_tag);
  }

  [[nodiscard]] z3::expr get_cell_assertion_binary(z3::context& ctx, const z3::expr& input1, const z3::expr& input2, const z3::expr& output) const override {
    uint32_t bv_size = output.get_sort().bv_size();
    // Z3_ast args[2] = {input1, input2};
    // unsigned largest_bit_size = (input1.get_sort().bv_size() < input2.get_sort().bv_size()) ? input2.get_sort().bv_size() : input1.get_sort().bv_size();
    unsigned largest_bit_size = std::max({input1.get_sort().bv_size(),
                                          input2.get_sort().bv_size(),
                                          output.get_sort().bv_size()});
    bool A_signed = false;
    const auto& jsign_A = m_parameters["A_SIGNED"];
    int sign_A = 0;
    if (jsign_A.is_number_integer()) {
      sign_A = jsign_A.get<int>();
    } else {
      sign_A = std::stoi(jsign_A.get<std::string>(), nullptr, 2);
    }
    Ensures(sign_A == 0 || sign_A == 1);
    A_signed = sign_A != 0;
    bool B_signed = false;
    const auto& jsign_B = m_parameters["B_SIGNED"];
    int sign_B = 0;
    if (jsign_B.is_number_integer()) {
      sign_B = jsign_B.get<int>();
    } else {
      sign_B = std::stoi(jsign_B.get<std::string>(), nullptr, 2);
    }
    Ensures(sign_B == 0 || sign_B == 1);
    B_signed = sign_B != 0;
    bool Result_signed = A_signed && B_signed;// According to Sec. 11.7, 1800-2012 - IEEE Standard for SystemVerilog the result is only signed, if all operands are signed.
    // The result of an expression is propagated to all operands.
    // TODO: Add tests for different bitwidths and signedness
    // TODO: Move decision about signedness to central place in cell class
    // SMT has a different semantics - bitvectors are just bitvectors. Their interpretation is done by operators
    //    - partial treatment in sshr ...
    z3::expr extended_input1 = input1;
    if (largest_bit_size > input1.get_sort().bv_size()) {
      extended_input1 = A_signed ? z3::sext(input1, largest_bit_size - input1.get_sort().bv_size()) : z3::zext(input1, largest_bit_size - input1.get_sort().bv_size());
    }
    z3::expr extended_input2 = input2;
    if (largest_bit_size > input2.get_sort().bv_size()) {
      extended_input2 = B_signed ? z3::sext(input2, largest_bit_size - input2.get_sort().bv_size()) : z3::zext(input2, largest_bit_size - input2.get_sort().bv_size());
    }

    z3::expr extended_output = output;
    if (largest_bit_size > output.get_sort().bv_size()) {
      extended_output = (!A_signed || !B_signed) ? z3::zext(output, largest_bit_size - output.get_sort().bv_size()) : z3::sext(output, largest_bit_size - output.get_sort().bv_size());
    }

    if (m_type == "$xor") {
      return (extended_output == (extended_input1 ^ extended_input2));
    }
    if (m_type == "$xnor") {
      return (extended_output == ~(extended_input1 ^ extended_input2));
    }
    if (m_type == "$add") {
      return (extended_output == (extended_input1 + extended_input2));
    }
    if (m_type == "$sub") {
      return (extended_output == (extended_input1 - extended_input2));
    }
    if (m_type == "$mul") {
      return (extended_output == (extended_input1 * extended_input2));
    }
    if (m_type == "$div") {
      if ((!A_signed && B_signed) || (A_signed && !B_signed)) {
        throw std::runtime_error("ge operation needs either two signed or two unsigned bitvectors!");
      }
      z3::expr comparison_condition = (A_signed && B_signed) ? z3::to_expr(ctx, extended_input1 / extended_input2) : z3::udiv(extended_input1, extended_input2);
      return (output == comparison_condition);
      // return (output == (extended_input1 / extended_input2));
    }
    if (m_type == "$eq") {
      // return (output == to_expr(ctx, Z3_mk_eq(ctx, input1, input2)));
      return (output == to_expr(ctx, Z3_mk_ite(ctx, (extended_input1 == extended_input2), ctx.bv_val(1, bv_size), ctx.bv_val(0, bv_size))));
    }
    if (m_type == "$eqx") {
      return (output == to_expr(ctx, Z3_mk_ite(ctx, (extended_input1 == extended_input2), ctx.bv_val(1, bv_size), ctx.bv_val(0, bv_size))));
    }
    if (m_type == "$ge") {
      if ((!A_signed && B_signed) || (A_signed && !B_signed)) {
        throw std::runtime_error("ge operation needs either two signed or two unsigned bitvectors!");
      }
      z3::expr comparison_condition = (A_signed && B_signed) ? z3::to_expr(ctx, Z3_mk_bvsge(ctx, extended_input1, extended_input2)) : z3::to_expr(ctx, Z3_mk_bvuge(ctx, extended_input1, extended_input2));
      return (output == to_expr(ctx, Z3_mk_ite(ctx, comparison_condition, ctx.bv_val(1, bv_size), ctx.bv_val(0, bv_size))));
    }
    if (m_type == "$gt") {
      if ((!A_signed && B_signed) || (A_signed && !B_signed)) {
        throw std::runtime_error("ge operation needs either two signed or two unsigned bitvectors!");
      }
      z3::expr comparison_condition = (A_signed && B_signed) ? z3::to_expr(ctx, Z3_mk_bvsgt(ctx, extended_input1, extended_input2)) : z3::to_expr(ctx, Z3_mk_bvugt(ctx, extended_input1, extended_input2));
      return (output == to_expr(ctx, Z3_mk_ite(ctx, comparison_condition, ctx.bv_val(1, bv_size), ctx.bv_val(0, bv_size))));
    }
    if (m_type == "$le") {
      if ((!A_signed && B_signed) || (A_signed && !B_signed)) {
        throw std::runtime_error("ge operation needs either two signed or two unsigned bitvectors!");
      }
      z3::expr comparison_condition = (A_signed && B_signed) ? z3::to_expr(ctx, Z3_mk_bvsle(ctx, extended_input1, extended_input2)) : z3::to_expr(ctx, Z3_mk_bvule(ctx, extended_input1, extended_input2));
      return (output == to_expr(ctx, Z3_mk_ite(ctx, comparison_condition, ctx.bv_val(1, bv_size), ctx.bv_val(0, bv_size))));
    }
    if (m_type == "$lt") {
      if ((!A_signed && B_signed) || (A_signed && !B_signed)) {
        throw std::runtime_error("ge operation needs either two signed or two unsigned bitvectors!");
      }
      z3::expr comparison_condition = (A_signed && B_signed) ? z3::to_expr(ctx, Z3_mk_bvslt(ctx, extended_input1, extended_input2)) : z3::to_expr(ctx, Z3_mk_bvult(ctx, extended_input1, extended_input2));
      return (output == to_expr(ctx, Z3_mk_ite(ctx, comparison_condition, ctx.bv_val(1, bv_size), ctx.bv_val(0, bv_size))));
    }
    if (m_type == "$mod") {
      if (!Result_signed) {// The mod operator is signed in SMT, so we need to make the bitvectors unsigned for unsigned results
        return (output == (z3::zext(extended_input1, 1) % z3::zext(extended_input2, 1)).extract(largest_bit_size - 1, 0));
      }
      return (output == (extended_input1 % extended_input2));
    }
    if (m_type == "$ne") {
      return (output == to_expr(ctx, Z3_mk_ite(ctx, (extended_input1 != extended_input2), ctx.bv_val(1, bv_size), ctx.bv_val(0, bv_size))));
    }
    if (m_type == "$nex") {
      return (output == to_expr(ctx, Z3_mk_ite(ctx, (extended_input1 != extended_input2), ctx.bv_val(1, bv_size), ctx.bv_val(0, bv_size))));
    }
    if (m_type == "$pow") {
      throw std::runtime_error("power operation not supported on bit vector expressions!");
      // return (output == to_expr(ctx, Z3_mk_power(ctx, input1, input2)));
    }
    if (m_type == "$shl") {
      return (output == to_expr(ctx, Z3_mk_bvshl(ctx, extended_input1, extended_input2)));
    }
    if (m_type == "$shr") {
      return (output == to_expr(ctx, Z3_mk_bvlshr(ctx, extended_input1, extended_input2)));
    }
    if (m_type == "$sshl") {
      return (output == to_expr(ctx, Z3_mk_bvshl(ctx, extended_input1, extended_input2)));
    }
    if (m_type == "$sshr") {
      if (largest_bit_size == 1) {
        z3::expr extended_input1_sshr = A_signed ? z3::sext(input1, 2 - input1.get_sort().bv_size()) : z3::zext(input1, 2 - input1.get_sort().bv_size());
        z3::expr extended_input2_sshr = B_signed ? z3::sext(input2, 2 - input2.get_sort().bv_size()) : z3::zext(input2, 2 - input2.get_sort().bv_size());
        z3::expr extended_output_sshr = (!A_signed || !B_signed) ? z3::zext(output, 2 - output.get_sort().bv_size()) : z3::sext(output, 2 - output.get_sort().bv_size());
        z3::expr output_sshr = to_expr(ctx, Z3_mk_bvashr(ctx, extended_input1_sshr, extended_input2_sshr));
        return (output == output_sshr.extract(0, 0));
      }
      if (!Result_signed) {
        return (output == to_expr(ctx, Z3_mk_bvlshr(ctx, extended_input1, extended_input2)));
      }
      return (output == to_expr(ctx, Z3_mk_bvashr(ctx, extended_input1, extended_input2)));
    }
    throw std::runtime_error("Unknown type.");
  }
};
}// namespace ducode
