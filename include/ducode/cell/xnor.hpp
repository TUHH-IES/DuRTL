/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/cell/basic_gate.hpp>

namespace ducode {

/**
 * @brief Represents a XNOR gate cell.
 * 
 * This class inherits from the CellBasicGateBinary class and provides
 * functionality specific to XNOR gates.
 */
class CellXnor : public CellBasicGateBinary {
public:
  /**
   * @brief Constructs a CellXnor object.
   * 
   * @param name The name of the cell.
   * @param type The type of the cell.
   * @param ports The ports of the cell.
   * @param hidden Indicates if the cell is hidden.
   * @param parameters The parameters of the cell.
   * @param attributes The attributes of the cell.
   */
  CellXnor(std::string name, std::string type, std::vector<ducode::Port>& ports, bool hidden, const nlohmann::json& parameters, const nlohmann::json& attributes)
      : CellBasicGateBinary(std::move(name), std::move(type), ports, hidden, parameters, attributes) {
  }

  /**
   * @brief Gets the Verilog operator representation of the XNOR gate.
   * 
   * @return The Verilog operator representation of the XNOR gate ("~^").
   */
  [[nodiscard]] std::string get_verilog_operator() const override {
    return "~^";
  }
};

}// namespace ducode