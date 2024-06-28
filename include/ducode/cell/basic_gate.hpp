/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/cell.hpp>

#include <utility>

namespace ducode {

class CellBasicGate : public Cell {
public:
  CellBasicGate(std::string name, std::string type, std::vector<ducode::Port>& ports, bool hidden, const nlohmann::json& parameters, const nlohmann::json& attributes)
      : Cell(std::move(name), std::move(type), ports, hidden, parameters, attributes) {
    assert(std::ranges::count_if(ports, [](const auto& port) { return port.m_direction == ducode::Port::Direction::output; }) == 1);
  }

  [[nodiscard]] virtual std::string get_verilog_operator() const = 0;

  /// CellBasicGate calls a general export function for basicgates which have a common verilog syntax and mostly different because of the used operator
  [[nodiscard]] std::string export_verilog(const nlohmann::json& params) const override {
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

    if (m_ports.size() == 2) {
      result << fmt::format("  assign {} = {}{};\n", y, get_verilog_operator(), a);
      if (ift) {
        result << ift_assignment(y, {a});
      }
    } else if (m_ports.size() == 3) {
      result << fmt::format("  assign {} = {} {} {};\n", y, a, get_verilog_operator(), b);
      if (ift) {
        result << ift_assignment(y, {a, b});
      }
    } else {
      throw std::runtime_error("more than 3 ports in a basic gate");
    }

    return result.str();
  }

  [[nodiscard]] virtual std::string ift_assignment([[maybe_unused]] const std::string& successor, [[maybe_unused]] const std::vector<std::string>& predecessors) const = 0;
};

class CellBasicGateUnary : public CellBasicGate {
public:
  CellBasicGateUnary(std::string name, std::string type, std::vector<ducode::Port>& ports, bool hidden, const nlohmann::json& parameters, const nlohmann::json& attributes)
      : CellBasicGate(std::move(name), std::move(type), ports, hidden, parameters, attributes) {
  }

  [[nodiscard]] std::string get_verilog_operator() const override {
    if (m_type == "$logic_not") {
      return "!";
    }
    if (m_type == "$not") {
      return "~";
    }
    if (m_type == "$pos") {
      return "";
    }
    if (m_type == "$neg") {
      return "-";
    }

    throw std::runtime_error("Unknown type.");
  }

  [[nodiscard]] std::string ift_assignment(const std::string& successor, const std::vector<std::string>& predecessors) const override {
    assert(!successor.empty());
    assert(predecessors.size() == 1);

    static constexpr std::string_view s = "assign {}{} = {}{} ;\n";
    return fmt::format(s, successor, ift_tag, predecessors[0], ift_tag);
  }
};

class CellBasicGateBinary : public CellBasicGate {
public:
  CellBasicGateBinary(std::string name, std::string type, std::vector<ducode::Port>& ports, bool hidden, const nlohmann::json& parameters, const nlohmann::json& attributes)
      : CellBasicGate(std::move(name), std::move(type), ports, hidden, parameters, attributes) {
  }

  [[nodiscard]] std::string get_verilog_operator() const override {
    if (m_type == "$add") {
      return "+";
    }
    if (m_type == "$sub") {
      return "-";
    }
    if (m_type == "$mul") {
      return "*";
    }
    if (m_type == "$div") {
      return "/";
    }
    if (m_type == "$eq") {
      return "==";
    }
    if (m_type == "$eqx") {
      return "===";
    }
    if (m_type == "$ge") {
      return ">=";
    }
    if (m_type == "$gt") {
      return ">";
    }
    if (m_type == "$le") {
      return "<=";
    }
    if (m_type == "$lt") {
      return "<";
    }
    if (m_type == "$mod") {
      return "%";
    }
    if (m_type == "$ne") {
      return "!=";
    }
    if (m_type == "$nex") {
      return "!==";
    }
    if (m_type == "$pow") {
      return "**";
    }
    if (m_type == "$shl") {
      return "<<";
    }
    if (m_type == "$shr") {
      return ">>";
    }
    if (m_type == "$sshl") {
      return "<<<";
    }
    if (m_type == "$sshr") {
      return ">>>";
    }

    throw std::runtime_error("Unknown type.");
  }

  [[nodiscard]] std::string ift_assignment(const std::string& successor, const std::vector<std::string>& predecessors) const override {
    assert(!successor.empty());
    assert(predecessors.size() == 2);

    static constexpr std::string_view s = "assign {}{} = {}{} | {}{} ;\n";
    return fmt::format(s, successor, ift_tag, predecessors[0], ift_tag, predecessors[1], ift_tag);
  }
};
}// namespace ducode
