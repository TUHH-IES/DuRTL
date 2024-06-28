/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/cell.hpp>
#include <ducode/port.hpp>

#include <cassert>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ducode {

/** \brief \c CellALDFFE represents a DFF register of a certain width with four predecessor nets;
pred_0= aDataIn, pred_1= aLoad, pred_2 = CLK, pred_3 = DataIn, pred_4 = ENable,  the succssor is the data output.
*/

class CellALDFFE : public Cell {
  bool clk_pos_edge;
  bool aload_active_high;
  bool en_active_high;

public:
  CellALDFFE(std::string name, std::string type, std::vector<ducode::Port>& ports, bool hidden, const nlohmann::json& parameters, const nlohmann::json& attributes)
      : Cell(std::move(name), std::move(type), ports, hidden, parameters, attributes) {
    // One output
    assert(ports.size() == 6);
    assert(ports[0].m_name == "AD");
    assert(ports[0].m_direction == ducode::Port::Direction::input);
    assert(ports[1].m_name == "ALOAD");
    assert(ports[1].m_direction == ducode::Port::Direction::input);
    assert(ports[2].m_name == "CLK");
    assert(ports[2].m_direction == ducode::Port::Direction::input);
    assert(ports[3].m_name == "D");
    assert(ports[3].m_direction == ducode::Port::Direction::input);
    assert(ports[4].m_name == "EN");
    assert(ports[4].m_direction == ducode::Port::Direction::input);
    assert(ports[5].m_name == "Q");
    assert(ports[5].m_direction == ducode::Port::Direction::output);
    assert(ports[1].m_bits.size() == 1);
    assert(ports[2].m_bits.size() == 1);
    assert(ports[4].m_bits.size() == 1);
    assert(ports[0].m_bits.size() == ports[5].m_bits.size());
    assert(ports[3].m_bits.size() == ports[5].m_bits.size());
    clk_pos_edge = std::stoull(parameters.at("CLK_POLARITY").get<std::string>(), nullptr, 2) != 0;
    aload_active_high = std::stoull(parameters.at("ALOAD_POLARITY").get<std::string>(), nullptr, 2) != 0;
    en_active_high = std::stoull(parameters.at("EN_POLARITY").get<std::string>(), nullptr, 2) != 0;
    m_register = true;
  }

  [[nodiscard]] std::string export_verilog(const nlohmann::json& params) const override {
    assert(m_ports.size() == 6);

    assert(params.contains("ift"));

    const bool ift = params["ift"].get<bool>();

    std::unordered_map<std::size_t, std::string> identifiers_map;
    if (params.contains("identifiers_map")) {
      identifiers_map = params["identifiers_map"].get<std::unordered_map<std::size_t, std::string>>();
    }

    std::stringstream result;
    std::string clk;
    std::string aload;
    std::string d;
    std::string ad;
    std::string q;
    std::string en;

    for (const auto& port: m_ports) {
      if (port.m_direction == ducode::Port::Direction::output) {
        assert(port.m_name == "Q");
        q = identifiers_map.at(port.m_bits_signature);
      }
      if (port.m_name == "CLK") {
        Ensures(port.m_direction == ducode::Port::Direction::input);
        clk = identifiers_map.at(port.m_bits_signature);
      }
      if (port.m_name == "D") {
        Ensures(port.m_direction == ducode::Port::Direction::input);
        d = identifiers_map.at(port.m_bits_signature);
      }
      if (port.m_name == "EN") {
        Ensures(port.m_direction == ducode::Port::Direction::input);
        en = identifiers_map.at(port.m_bits_signature);
      }
      if (port.m_name == "AD") {
        Ensures(port.m_direction == ducode::Port::Direction::input);
        ad = identifiers_map.at(port.m_bits_signature);
      }
      if (port.m_name == "ALOAD") {
        Ensures(port.m_direction == ducode::Port::Direction::input);
        aload = identifiers_map.at(port.m_bits_signature);
      }
    }

    std::string clk_edge = clk_pos_edge ? "posedge" : "negedge";
    std::string aload_edge = aload_active_high ? "posedge" : "negedge";
    std::string aload_neg = aload_active_high ? "" : "!";
    std::string enable_neg = en_active_high ? "" : "!";
    result << fmt::format("  always @({} {} or {} {}) begin\n", clk_edge, clk, aload_edge, aload)
           << fmt::format("      if ({}{}) begin\n", aload_neg, aload)
           << fmt::format("          {} <= {};\n", q, ad);
    if (ift) {
      result << fmt::format("          {}{} <= (^{} === 1'bx) ? 0 : (({} == {}) ? ({}{} | {}{}) : {}{});\n", q, ift_tag, d, d, aload, d, ift_tag, ad, ift_tag, ad, ift_tag);
    }
    result << fmt::format("      end else if ({}{}) begin\n", enable_neg, en);
    result << fmt::format("          {} <= {};\n", q, d);
    if (ift) {
      result << fmt::format("          {}{} <= (^{} === 1'bx) ? 0 : ((({} == {}) ? ({}{} | {}{}) : {}{}) | {}{});\n", q, ift_tag, d, d, aload, d, ift_tag, ad, ift_tag, d, ift_tag, en, ift_tag);
    }
    result << "       end\n";
    result << "  end\n\n";

    return result.str();
  }
};
}// namespace ducode