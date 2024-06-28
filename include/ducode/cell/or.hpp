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
class CellOr : public CellBasicGate {
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
      : CellBasicGate(std::move(name), std::move(type), ports, hidden, parameters, attributes) {
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
};
}// namespace ducode