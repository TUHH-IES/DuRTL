/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/cell.hpp>

namespace ducode {

class Print : public Cell {
public:
  Print(std::string name, std::string type, std::vector<ducode::Port>& ports, bool hidden, const nlohmann::json& parameters, const nlohmann::json& attributes)
      : Cell(std::move(name), std::move(type), ports, hidden, parameters, attributes) {
  }

  [[nodiscard]] std::string export_verilog([[maybe_unused]] const nlohmann::json& params) const override {
    return "";
  }
};
}// namespace ducode