/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/cell.hpp>

#include <utility>

namespace ducode {

/** \brief \c CellDLatchSR represents a DLATCHSR register of a certain width with 4 predecessor nets;
pred_0 = CLR, pred_1 = dataIn, pred_2 = ENable, pred_3 = SET, the succssor is the data output.*/

class CellDLatchSR : public Cell {
  bool enable_active_high;
  bool set_active_high;
  bool clr_active_high;

public:
  CellDLatchSR(std::string name, std::string type, std::vector<ducode::Port>& ports, bool hidden, const nlohmann::json& parameters, const nlohmann::json& attributes)
      : Cell(std::move(name), std::move(type), ports, hidden, parameters, attributes) {
    // One output
    assert(ports.size() == 5);
    assert(ports[0].m_name == "CLR");
    assert(ports[0].m_direction == ducode::Port::Direction::input);
    assert(ports[1].m_name == "D");
    assert(ports[1].m_direction == ducode::Port::Direction::input);
    assert(ports[2].m_name == "EN");
    assert(ports[2].m_direction == ducode::Port::Direction::input);
    assert(ports[3].m_name == "Q");
    assert(ports[3].m_direction == ducode::Port::Direction::output);
    assert(ports[4].m_name == "SET");
    assert(ports[4].m_direction == ducode::Port::Direction::input);
    assert(ports[2].m_bits.size() == 1);
    assert(ports[0].m_bits.size() == ports[3].m_bits.size());
    assert(ports[1].m_bits.size() == ports[3].m_bits.size());
    assert(ports[4].m_bits.size() == ports[3].m_bits.size());
    set_active_high = std::stoull(parameters.at("SET_POLARITY").get<std::string>(), nullptr, 2) != 0;
    clr_active_high = std::stoull(parameters.at("CLR_POLARITY").get<std::string>(), nullptr, 2) != 0;
    enable_active_high = std::stoull(parameters.at("EN_POLARITY").get<std::string>(), nullptr, 2) != 0;
    m_register = true;
  }

  [[nodiscard]] std::string export_verilog(const nlohmann::json& params) const override {
    assert(m_ports.size() == 5);

    assert(params.contains("ift"));

    const bool ift = params["ift"].get<bool>();

    std::unordered_map<std::size_t, std::string> identifiers_map;
    if (params.contains("identifiers_map")) {
      identifiers_map = params["identifiers_map"].get<std::unordered_map<std::size_t, std::string>>();
    }

    std::stringstream result;

    std::string en;
    std::string set;
    std::string clr;
    std::string d;
    std::string q;

    std::size_t output_port_size = 0;

    for (const auto& port: m_ports) {
      if (port.m_direction == ducode::Port::Direction::output) {
        assert(port.m_name == "Q");
        q = identifiers_map.at(port.m_bits_signature);
        output_port_size = port.m_bits.size();
      }
      if (port.m_name == "D") {
        Ensures(port.m_direction == ducode::Port::Direction::input);
        d = identifiers_map.at(port.m_bits_signature);
      }
      if (port.m_name == "EN") {
        Ensures(port.m_direction == ducode::Port::Direction::input);
        en = identifiers_map.at(port.m_bits_signature);
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

    std::string neg_en = enable_active_high ? "" : "!";
    std::string neg_set = set_active_high ? "" : "!";
    std::string neg_clr = clr_active_high ? "" : "!";

    if (output_port_size > 1) {
      for (auto i = 0ul; i < output_port_size; i++) {
        result << "always @* begin \n";
        result << fmt::format("    if ({}{}[{}]) begin \n", neg_clr, clr, i);
        result << fmt::format("        {}[{}] <= 0;\n", q, i);
        if (ift) {
          result << fmt::format("        {}{} <= {}{};\n", q, ift_tag, clr, ift_tag);
        }
        result << fmt::format("    end else if ({}{}[{}]) begin \n", neg_set, set, i);
        result << fmt::format("        {}[{}] <= 1;\n", q, i);
        if (ift) {
          result << fmt::format("        {}{} <= {}{};\n", q, ift_tag, set, ift_tag);
        }
        result << fmt::format("    end else if ({}{}) begin \n", neg_en, en);
        result << fmt::format("        {}[{}] <= {}[{}];\n", q, i, d, i);
        if (ift) {
          result << fmt::format("        {}{} <= {}{} | {}{};\n", q, ift_tag, d, ift_tag, en, ift_tag);
        }
        result << "     end\n end\n";
      }
    } else {
      result << "always @* begin \n";
      result << fmt::format("    if ({}{}) begin\n", neg_clr, clr);
      result << fmt::format("        {} <= 0;\n", q);
      if (ift) {
        result << fmt::format("        {}{} <= {}{};\n", q, ift_tag, clr, ift_tag);
      }
      result << fmt::format("    end else if ({}{}) begin\n", neg_set, set);
      result << fmt::format("        {} <= 1;\n", q);
      if (ift) {
        result << fmt::format("        {}{} <= {}{};\n", q, ift_tag, set, ift_tag);
      }
      result << fmt::format("    end else if ({}{}) begin\n", neg_en, en);
      result << fmt::format("        {} <= {};\n", q, d);
      if (ift) {
        result << fmt::format("        {}{} <= {}{} | {}{};\n", q, ift_tag, d, ift_tag, en, ift_tag);
      }
      result << "   end\nend\n\n";
    }

    return result.str();
  }
};
}// namespace ducode