/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/cell/basic_gate.hpp>

namespace ducode {
class CellReduceXor : public CellBasicGateUnary {
public:
  CellReduceXor(std::string name, std::string type, std::vector<ducode::Port>& ports, bool hidden, const nlohmann::json& parameters, const nlohmann::json& attributes)
      : CellBasicGateUnary(std::move(name), std::move(type), ports, hidden, parameters, attributes) {
  }

  [[nodiscard]] std::string get_verilog_operator() const override {
    return "^";
  }

  [[nodiscard]] z3::expr get_cell_assertion_unary(z3::context& ctx, const z3::expr& input1, const z3::expr& output) const override {
    if (output.get_sort().bv_size() == 1) {
      return (output == input1);
    }
    if (output.get_sort().bv_size() != 1) {
      std::vector<Z3_ast> arr;
      for (uint32_t i = 0; i < output.get_sort().bv_size(); i++) {
        arr.push_back(Z3_mk_extract(ctx, i, i, input1));
      }
      return (output == to_expr(ctx, Z3_mk_add(ctx, gsl::narrow<uint32_t>(arr.size()), arr.data())));
    }
    throw std::runtime_error("Should never reach here");
  }
};
}// namespace ducode