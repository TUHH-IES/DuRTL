/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/cell/basic_gate.hpp>

namespace ducode {

/**
 * @brief Represents a XOR gate cell.
 * 
 * This class inherits from CellBasicGateBinary and provides the implementation for a XOR gate cell.
 */
class CellXor : public CellBasicGateBinary {
public:
  /**
   * @brief Constructs a CellXor object.
   * 
   * @param name The name of the cell.
   * @param type The type of the cell.
   * @param ports The vector of ports associated with the cell.
   * @param hidden A flag indicating whether the cell is hidden or not.
   * @param parameters The JSON object containing the parameters of the cell.
   * @param attributes The JSON object containing the attributes of the cell.
   */
  CellXor(std::string name, std::string type, std::vector<ducode::Port>& ports, bool hidden, const nlohmann::json& parameters, const nlohmann::json& attributes)
      : CellBasicGateBinary(std::move(name), std::move(type), ports, hidden, parameters, attributes) {
  }

  /**
   * @brief Returns the Verilog operator for the XOR gate.
   * 
   * @return The Verilog operator for the XOR gate ("^").
   */
  [[nodiscard]] std::string get_verilog_operator() const override {
    return "^";
  }
};
}// namespace ducode