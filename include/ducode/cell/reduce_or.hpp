/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/cell/basic_gate.hpp>

namespace ducode {

/**
 * @brief Represents a cell that performs a logical OR operation on its input.
 * 
 * This class inherits from the CellBasicGateUnary class and overrides the get_verilog_operator() function to return the OR operator "|".
 */
class CellReduceOr : public CellBasicGateUnary {
public:
  /**
   * @brief Constructs a CellReduceOr object.
   * 
   * @param name The name of the cell.
   * @param type The type of the cell.
   * @param ports The ports of the cell.
   * @param hidden Indicates whether the cell is hidden.
   * @param parameters The parameters of the cell.
   * @param attributes The attributes of the cell.
   */
  CellReduceOr(std::string name, std::string type, std::vector<ducode::Port>& ports, bool hidden, const nlohmann::json& parameters, const nlohmann::json& attributes)
      : CellBasicGateUnary(std::move(name), std::move(type), ports, hidden, parameters, attributes) {
  }

  /**
   * @brief Returns the Verilog operator for the logical OR operation.
   * 
   * @return The Verilog operator "|".
   */
  [[nodiscard]] std::string get_verilog_operator() const override {
    return "|";
  }
};
}// namespace ducode