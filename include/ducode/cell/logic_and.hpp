/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/cell/basic_gate.hpp>

#include <utility>

namespace ducode {
class CellLogicAnd : public CellBasicGateBinary {
public:
  CellLogicAnd(std::string name, std::string type, std::vector<ducode::Port>& ports, bool hidden, const nlohmann::json& parameters, const nlohmann::json& attributes)
      : CellBasicGateBinary(std::move(name), std::move(type), ports, hidden, parameters, attributes) {
  }

  [[nodiscard]] std::string get_verilog_operator() const override {
    return "&&";
  }

  [[nodiscard]] std::string ift_assignment(const std::string& successor, const std::vector<std::string>& predecessors) const override {
    assert(!successor.empty());
    assert(predecessors.size() == 2);

    static constexpr std::string_view s = "assign {}{} = (^{} === 1'bx ? 0 : ({} > 0 ? {}{} : 0)) | (^{} === 1'bx ? 0 : ({} > 0 ? {}{} : 0)) | ({}{} & {}{});\n";
    return fmt::format(s, successor, ift_tag, predecessors[0], predecessors[0], predecessors[1], ift_tag, predecessors[1], predecessors[1], predecessors[0], ift_tag, predecessors[0], ift_tag, predecessors[1], ift_tag);
    //    if (false) {
    //      static constexpr std::string_view s = "assign {}{} = ({}{} | {}{});\n";
    //      return fmt::format(s, successor, ift_tag, predecessors[0], ift_tag, predecessors[1], ift_tag);
    //    }
  }

  [[nodiscard]] z3::expr get_cell_assertion_binary(z3::context& ctx, const z3::expr& input1, const z3::expr& input2, const z3::expr& output) const override {
    uint32_t bv_size = output.get_sort().bv_size();
    return (output == to_expr(ctx, Z3_mk_ite(ctx, ((input1 != 0) && (input2 != 0)), ctx.bv_val(1, bv_size), ctx.bv_val(0, bv_size))));
  }
};
}// namespace ducode