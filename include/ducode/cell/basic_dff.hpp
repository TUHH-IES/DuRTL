/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/cell.hpp>

#include <utility>

namespace ducode {

class CellBasicDFF : public Cell {
protected:
  //NOLINTBEGIN(*-non-private-member-variables-in-classes)
  bool clk_pos_edge{};
  uint64_t width{};

  //NOLINTEND(*-non-private-member-variables-in-classes)

public:
  CellBasicDFF(std::string name, std::string type, std::vector<ducode::Port>& ports, bool hidden, const nlohmann::json& parameters, const nlohmann::json& attributes)
      : Cell(std::move(name), std::move(type), ports, hidden, parameters, attributes) {
    clk_pos_edge = std::stoull(parameters.at("CLK_POLARITY").get<std::string>(), nullptr, 2) != 0;
    width = std::stoull(parameters.at("WIDTH").get<std::string>(), nullptr, 2);
    m_register = true;
  }
};
}// namespace ducode