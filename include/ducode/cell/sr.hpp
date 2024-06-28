/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/cell.hpp>

namespace ducode {

/** \brief \c CellSR represents a register of a certain width with two predecessor nets;
    pred_0= SET, pred_1= CLR, the succssor is the data output Q.
*/
class CellSR : public Cell {
  bool set_active_high;
  bool clr_active_high;

public:
  CellSR(std::string name, std::string type, std::vector<ducode::Port>& ports, bool hidden, const nlohmann::json& parameters, const nlohmann::json& attributes)
      : Cell(std::move(name), std::move(type), ports, hidden, parameters, attributes) {
    // One output
    assert(ports.size() == 3);
    assert(ports[0].m_name == "CLR");
    assert(ports[0].m_direction == ducode::Port::Direction::input);
    assert(ports[1].m_name == "Q");
    assert(ports[1].m_direction == ducode::Port::Direction::output);
    assert(ports[2].m_name == "SET");
    assert(ports[2].m_direction == ducode::Port::Direction::input);
    assert(ports[0].m_bits.size() == ports[1].m_bits.size());
    assert(ports[0].m_bits.size() == ports[2].m_bits.size());

    clr_active_high = std::stoull(parameters.at("CLR_POLARITY").get<std::string>(), nullptr, 2) != 0;
    set_active_high = std::stoull(parameters.at("SET_POLARITY").get<std::string>(), nullptr, 2) != 0;
    m_register = true;
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
    std::string set;
    std::string clr;
    std::string q;

    for (const auto& port: m_ports) {
      if (port.m_direction == ducode::Port::Direction::output) {
        assert(port.m_name == "Q");
        q = identifiers_map.at(port.m_bits_signature);
      }
      if (port.m_name == "SET") {
        Ensures(port.m_direction == ducode::Port::Direction::input);
        set = identifiers_map.at(port.m_bits_signature);
      }
      if (port.m_name == "CLR") {
        Ensures(port.m_direction == ducode::Port::Direction::input);
        clr = identifiers_map.at(port.m_bits_signature);
      }
    }

    std::string neg_set = set_active_high ? "" : "!";
    std::string neg_clr = clr_active_high ? "" : "!";

    for (uint32_t i = 0; i < m_ports[0].m_bits.size(); i++) {
      result << "always @* begin\n"
             << fmt::format(" if({}{}[{}])begin\n", neg_clr, clr, i)
             << fmt::format("   {}[{}] = 0;\n", q, i);
      if (ift) {
        result << fmt::format("   {}{} = {}{};\n", q, ift_tag, clr, ift_tag);
      }
      result << fmt::format("end else if ({}{}[{}]) begin\n", neg_set, set, i)
             << fmt::format("   {}[{}] = 1;\n", q, i);
      if (ift) {
        result << fmt::format("   {}{} = {}{};\n", q, ift_tag, set, ift_tag);
      }
      result << "end end\n\n";
    }

    return result.str();
  }
};
}// namespace ducode