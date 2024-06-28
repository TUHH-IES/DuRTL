/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/cell/basic_gate.hpp>

/** \brief \c CellShift represents a shift register with two predecessor nets;
*  the data input A, the shift direction B;
*  the successor is the data output Y;
*/

namespace ducode {

class CellShift : public Cell {
  bool A_signed;
  bool B_signed;

public:
  CellShift(std::string name, std::string type, std::vector<ducode::Port>& ports, bool hidden, const nlohmann::json& parameters, const nlohmann::json& attributes)
      : Cell(std::move(name), std::move(type), ports, hidden, parameters, attributes) {
    // One output
    assert(ports.size() == 3);
    assert(ports[0].m_name == "A");
    assert(ports[0].m_direction == ducode::Port::Direction::input);
    assert(ports[1].m_name == "B");
    assert(ports[1].m_direction == ducode::Port::Direction::input);
    assert(ports[2].m_name == "Y");
    assert(ports[2].m_direction == ducode::Port::Direction::output);
    A_signed = ports[0].m_signed;
    B_signed = ports[1].m_signed;
  }

  [[nodiscard]] std::string export_verilog(const nlohmann::json& params) const override {
    assert(m_ports.size() == 3);

    assert(params.contains("ift"));

    const bool ift = params["ift"].get<bool>();

    std::unordered_map<std::size_t, std::string> identifiers_map;
    if (params.contains("identifiers_map")) {
      identifiers_map = params["identifiers_map"].get<std::unordered_map<std::size_t, std::string>>();
    }

    std::stringstream result;
    std::string a;
    std::string b;
    std::string y;

    for (const auto& port: m_ports) {
      if (port.m_direction == ducode::Port::Direction::output) {
        assert(port.m_name == "Y");
        y = identifiers_map.at(port.m_bits_signature);
      }
      if (port.m_name == "A") {
        Ensures(port.m_direction == ducode::Port::Direction::input);
        a = identifiers_map.at(port.m_bits_signature);
      }
      if (port.m_name == "B") {
        Ensures(port.m_direction == ducode::Port::Direction::input);
        b = identifiers_map.at(port.m_bits_signature);
      }
    }

    if (A_signed) {
      if (B_signed) {
        result << fmt::format("  assign {} = $signed({}) < 0 ? $signed({}) << -{}: $signed({}) >> {};\n", y, b, a, b, a, b);
      } else {
        result << fmt::format("  assign {} = $signed({}) >> {};\n", y, a, b);
      }
    } else {
      if (B_signed) {
        result << fmt::format("  assign {} = $signed({}) < 0 ? {} << -{}: {} >> {};\n", y, b, a, b, a, b);
      } else {
        result << fmt::format("  assign {} = {} >> {};\n", y, a, b);
      }
    }

    if (ift) {
      result << fmt::format("  assign {} = {} | {};\n", y + ift_tag, a + ift_tag, b + ift_tag);
    }

    return result.str();
  }
};
}// namespace ducode