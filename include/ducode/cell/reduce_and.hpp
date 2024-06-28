/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/cell/basic_gate.hpp>
#include <ducode/port.hpp>

#include <string>
#include <utility>
#include <vector>

namespace ducode {

class CellReduceAnd : public CellBasicGateUnary {
public:
  CellReduceAnd(std::string name, std::string type, std::vector<ducode::Port>& ports, bool hidden, const nlohmann::json& parameters, const nlohmann::json& attributes)
      : CellBasicGateUnary(std::move(name), std::move(type), ports, hidden, parameters, attributes) {
  }

  [[nodiscard]] std::string get_verilog_operator() const override {
    return "&";
  }
};
}// namespace ducode