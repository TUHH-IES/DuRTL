/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/cell/basic_gate.hpp>

namespace ducode {

/**
 * @brief Represents a logical OR gate cell.
 * 
 * This class inherits from the CellBasicGate class and provides
 * functionality specific to a logical OR gate.
 */
class CellOr : public CellBasicGateBinary {
public:
  /**
   * @brief Constructs a CellOr object.
   * 
   * @param name The name of the cell.
   * @param type The type of the cell.
   * @param ports The ports of the cell.
   * @param hidden Indicates if the cell is hidden.
   * @param parameters The parameters of the cell.
   * @param attributes The attributes of the cell.
   */
  CellOr(std::string name, std::string type, std::vector<ducode::Port>& ports, bool hidden, const nlohmann::json& parameters, const nlohmann::json& attributes)
      : CellBasicGateBinary(std::move(name), std::move(type), ports, hidden, parameters, attributes) {
  }

  /**
   * @brief Gets the Verilog operator for the OR gate.
   * 
   * @return The Verilog operator for the OR gate ("|").
   */
  [[nodiscard]] std::string get_verilog_operator() const override {
    return "|";
  }

  /**
   * @brief Generates the IFT assignment for the OR gate.
   * 
   * @param successor The successor of the OR gate.
   * @param predecessors The predecessors of the OR gate.
   * @return The generated IFT assignment string.
   */
  [[nodiscard]] std::string ift_assignment(const std::string& successor, const std::vector<std::string>& predecessors) const override {
    assert(!successor.empty());
    assert(predecessors.size() == 2);
    static constexpr std::string_view s = "assign {}{} = (^{} === 1'bx ? 0 : ({} == 0 ? {}{} : 0)) | (^{} === 1'bx ? 0 : ({} == 0 ? {}{} : 0)) | ({}{} & {}{}) ;\n";
    return fmt::format(s, successor, ift_tag, predecessors[0], predecessors[0], predecessors[1], ift_tag, predecessors[1], predecessors[1], predecessors[0], ift_tag, predecessors[0], ift_tag, predecessors[1], ift_tag);
  }

  [[nodiscard]] z3::expr get_cell_assertion_binary([[maybe_unused]] z3::context& ctx, const z3::expr& input1, const z3::expr& input2, const z3::expr& output) const override {
    unsigned largest_bit_size = (input1.get_sort().bv_size() < input2.get_sort().bv_size()) ? input2.get_sort().bv_size() : input1.get_sort().bv_size();
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
    z3::expr extended_input1 = A_signed ? z3::sext(input1, largest_bit_size - input1.get_sort().bv_size()) : z3::zext(input1, largest_bit_size - input1.get_sort().bv_size());
    z3::expr extended_input2 = B_signed ? z3::sext(input2, largest_bit_size - input2.get_sort().bv_size()) : z3::zext(input2, largest_bit_size - input2.get_sort().bv_size());


    return (output == (extended_input1 | extended_input2));
  }
};
}// namespace ducode