/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/cell.hpp>

#include <utility>

namespace ducode {

/** \brief \c CellDFFSR represents a DFFSR register of a certain width with four predecessor nets;
    pred_0= CLK, pred_1= CLR, pred_2 = dataIn, pred_3 = SET the succssor is the data output.
*/

class CellDFFSR : public Cell {
  bool clk_pos_edge;
  bool set_active_high;
  bool clr_active_high;

public:
  CellDFFSR(std::string name, std::string type, std::vector<ducode::Port>& ports, bool hidden, const nlohmann::json& parameters, const nlohmann::json& attributes)
      : Cell(std::move(name), std::move(type), ports, hidden, parameters, attributes) {
    // One output
    assert(ports.size() == 5);
    assert(ports[0].m_name == "CLK");
    assert(ports[0].m_direction == ducode::Port::Direction::input);
    assert(ports[1].m_name == "CLR");
    assert(ports[1].m_direction == ducode::Port::Direction::input);
    assert(ports[2].m_name == "D");
    assert(ports[2].m_direction == ducode::Port::Direction::input);
    assert(ports[3].m_name == "Q");
    assert(ports[3].m_direction == ducode::Port::Direction::output);
    assert(ports[4].m_name == "SET");
    assert(ports[4].m_direction == ducode::Port::Direction::input);
    assert(ports[0].m_bits.size() == 1);
    assert(ports[1].m_bits.size() == ports[3].m_bits.size());
    assert(ports[2].m_bits.size() == ports[3].m_bits.size());
    assert(ports[4].m_bits.size() == ports[3].m_bits.size());
    clk_pos_edge = std::stoull(parameters.at("CLK_POLARITY").get<std::string>(), nullptr, 2) != 0;
    set_active_high = std::stoull(parameters.at("SET_POLARITY").get<std::string>(), nullptr, 2) != 0;
    clr_active_high = std::stoull(parameters.at("CLR_POLARITY").get<std::string>(), nullptr, 2) != 0;
    m_register = true;
  }

  [[nodiscard]] std::string export_verilog(const nlohmann::json& params) const override {
    assert(params.contains("ift"));

    bool ift = params["ift"].get<bool>();

    std::unordered_map<std::size_t, std::string> identifiers_map;
    if (params.contains("identifiers_map")) {
      identifiers_map = params["identifiers_map"].get<std::unordered_map<std::size_t, std::string>>();
    }

    std::stringstream result;
    std::string clk;
    std::string set;
    std::string d;
    std::string clr;
    std::string q;

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
      if (port.m_name == "CLR") {
        Ensures(port.m_direction == ducode::Port::Direction::input);
        clr = identifiers_map.at(port.m_bits_signature);
      }
      if (port.m_name == "SET") {
        Ensures(port.m_direction == ducode::Port::Direction::input);
        set = identifiers_map.at(port.m_bits_signature);
      }
    }

    std::string clk_edge = clk_pos_edge ? "posedge" : "negedge";
    std::string set_edge = set_active_high ? "posedge" : "negedge";
    std::string clr_edge = clr_active_high ? "posedge" : "negedge";
    std::string neg_set = set_active_high ? "" : "!";
    std::string neg_clr = clr_active_high ? "" : "!";

    auto output_port_it = std::ranges::find_if(m_ports, [](const ducode::Port& port) { return port.m_name == "Q" && port.m_direction == ducode::Port::Direction::output; });
    if (output_port_it == m_ports.end()) {
      throw std::runtime_error("cannot find output port");
    }

    if (output_port_it->m_bits.size() > 1) {
      for (uint32_t i = 0; i < output_port_it->m_bits.size(); i++) {
        result << fmt::format("  always @({} {} , {} {}[{}] , {} {}[{}]) begin\n", clk_edge, clk, set_edge, set, i, clr_edge, clr, i);
        result << fmt::format("      if ({}{}[{}]) begin\n", neg_clr, clr, i);
        result << fmt::format("          {}[{}] <= 0;\n", q, i);
        if (ift) {
          result << fmt::format("          {}{} <= {}{};\n", q, ift_tag, clr, ift_tag);
        }
        result << fmt::format("      end else if ({}{}[{}]) begin\n", neg_set, set, i);
        result << fmt::format("          {}[{}] <= 1;\n", q, i);
        if (ift) {
          result << fmt::format("          {}{} <= (^{} === 1'bx) ? 0 : (({} == {}) ? ({}{} | {}{}) : {}{});\n", q, ift_tag, d, d, set, d, ift_tag, set, ift_tag, set, ift_tag);
        }
        result << "          end\n"
               << "      else begin\n";
        result << fmt::format("          {}[{}] <= {}[{}];\n", q, i, d, i);
        if (ift) {
          result << fmt::format("          {}{} <= {}{};\n", q, ift_tag, d, ift_tag);
        }
        result << "       end\n";
        result << "  end\n\n";
      }
    } else {
      result << fmt::format("  always @({} {} , {} {} , {} {}) begin\n", clk_edge, clk, set_edge, set, clr_edge, clr);
      result << fmt::format("      if ({}{}) begin\n", neg_clr, clr)
             << fmt::format("          {} <= 0;\n", q);
      if (ift) {
        result << fmt::format("          {}{} <= {}{};\n", q, ift_tag, clr, ift_tag);
      }
      result << fmt::format("      end else if ({}{}) begin\n", neg_set, set)
             << fmt::format("          {} <= 1;\n", q);
      if (ift) {
        result << fmt::format("          {}{} <= (^{} === 1'bx) ? 0 : (({} == {}) ? ({}{} | {}{}) : {}{});\n", q, ift_tag, d, d, set, d, ift_tag, set, ift_tag, set, ift_tag);
      }
      result << "          end\n"
             << "      else begin\n";
      result << fmt::format("          {} <= {};\n", q, d);
      if (ift) {
        result << fmt::format("          {}{} <= {}{};\n", q, ift_tag, d, ift_tag);
      }
      result << "       end\n";
      result << "  end\n\n";
    }

    return result.str();
  }
};
}// namespace ducode