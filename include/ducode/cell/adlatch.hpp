/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/cell.hpp>

#include <utility>

namespace ducode {

/** \brief \c CellADLatch represents a D-Latch register of a certain width with three predecessor nets;
pred_0= dataIn, pred_1= ENable, pred_2 = aRST, the succssor is the data output.
*/

class CellADLatch : public Cell {
  bool enable_active_high;
  bool arst_active_high;
  uint64_t arst_value;

public:
  CellADLatch(std::string name, std::string type, std::vector<ducode::Port>& ports, bool hidden, const nlohmann::json& parameters, const nlohmann::json& attributes)
      : Cell(std::move(name), std::move(type), ports, hidden, parameters, attributes) {
    // ports need to be verified
    assert(ports.size() == 4);
    assert(ports[0].m_name == "ARST");
    assert(ports[1].m_name == "D");
    assert(ports[2].m_name == "EN");
    assert(ports[3].m_name == "Q");
    enable_active_high = std::stoull(parameters.at("EN_POLARITY").get<std::string>(), nullptr, 2) != 0;
    arst_active_high = std::stoull(parameters.at("ARST_POLARITY").get<std::string>(), nullptr, 2) != 0;
    arst_value = std::stoull(parameters.at("ARST_VALUE").get<std::string>(), nullptr, 2);
    m_register = true;
  }

  [[nodiscard]] std::string export_verilog(const nlohmann::json& params) const override {
    assert(params.contains("ift"));

    const bool ift = params["ift"].get<bool>();

    std::unordered_map<std::size_t, std::string> identifiers_map;
    if (params.contains("identifiers_map")) {
      identifiers_map = params["identifiers_map"].get<std::unordered_map<std::size_t, std::string>>();
    }

    std::stringstream result;
    std::string d;
    std::string q;
    std::string arst;
    std::string en;

    for (const auto& port: m_ports) {
      if (port.m_direction == ducode::Port::Direction::output) {
        assert(port.m_name == "Q");
        q = identifiers_map.at(port.m_bits_signature);
      }
      if (port.m_name == "D") {
        Ensures(port.m_direction == ducode::Port::Direction::input);
        d = identifiers_map.at(port.m_bits_signature);
      }
      if (port.m_name == "EN") {
        Ensures(port.m_direction == ducode::Port::Direction::input);
        en = identifiers_map.at(port.m_bits_signature);
      }
      if (port.m_name == "ARST") {
        Ensures(port.m_direction == ducode::Port::Direction::input);
        arst = identifiers_map.at(port.m_bits_signature);
      }
    }

    std::string neg_arst = arst_active_high ? "" : "!";
    std::string neg_en = enable_active_high ? "" : "!";

    result << "  always @* begin\n";
    result << fmt::format("    if ({}{})begin\n", neg_arst, arst);
    result << fmt::format("      {} <= {};\n", q, arst_value);
    if (ift) {
      result << fmt::format("          {}{} <= (^{} === 1'bx) ? 0 : (({} == {}) ? ({}{} | {}{}) : {}{});\n", q, ift_tag, d, d, arst_value, d, ift_tag, arst, ift_tag, arst, ift_tag);
    }
    result << fmt::format("   end else if ({}{}) begin\n", neg_en, en);
    result << fmt::format("      {} <= {};\n", q, d);
    if (ift) {
      result << fmt::format("          {}{} <= (^{} === 1'bx) ? 0 : ((({} == {}) ? ({}{} | {}{}) : {}{}) | {}{});\n", q, ift_tag, d, d, arst_value, d, ift_tag, arst, ift_tag, d, ift_tag, en, ift_tag);
    }
    result << "   end\n";
    result << "  end\n\n";

    return result.str();
  }
};
}// namespace ducode