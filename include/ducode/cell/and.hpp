/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/cell/basic_gate.hpp>

#include <utility>

namespace ducode {

class CellAnd : public CellBasicGateBinary {
public:
  CellAnd(std::string name, std::string type, std::vector<ducode::Port>& ports, bool hidden, const nlohmann::json& parameters, const nlohmann::json& attributes)
      : CellBasicGateBinary(std::move(name), std::move(type), ports, hidden, parameters, attributes) {
  }

  [[nodiscard]] std::string get_verilog_operator() const override {
    return "&";
  }

  [[nodiscard]] std::string ift_assignment(const std::string& successor, const std::vector<std::string>& predecessors) const override {
    assert(!successor.empty());
    assert(predecessors.size() == 2);

    static constexpr std::string_view s = "assign {y}{_t} = (^{a} === 1'bx ? 0 : ({a} > 0 ? {b}{_t} : 0)) | (^{b} === 1'bx ? 0 : ({b} > 0 ? {a}{_t} : 0)) | ({a}{_t} & {b}{_t});\n";
    return fmt::format(s, fmt::arg("y", successor), fmt::arg("_t", ift_tag), fmt::arg("a", predecessors[0]), fmt::arg("b", predecessors[1]));
  }

  [[nodiscard]] z3::expr get_cell_assertion_binary([[maybe_unused]] z3::context& ctx, const z3::expr& input1, const z3::expr& input2, const z3::expr& output) const override {

    return (output == (input1 & input2));
  }

  void export_smt2([[maybe_unused]] z3::context& ctx, z3::solver& solver, const std::unordered_map<std::string, z3::expr>& port_expr_map, [[maybe_unused]] const nlohmann::json& params) const override {
    assert(port_expr_map.size() <= 3);

    solver.add(port_expr_map.at("Y") == (port_expr_map.at("A") & port_expr_map.at("B")));
  }
};
}// namespace ducode
