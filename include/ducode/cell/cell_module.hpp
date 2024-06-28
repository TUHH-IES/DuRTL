/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/cell.hpp>
#include <ducode/port.hpp>
#include <ducode/utility/legalize_identifier.hpp>

#include <boost/algorithm/string/join.hpp>
#include <nlohmann/json.hpp>

#include <cassert>
#include <string>
#include <utility>
#include <vector>

namespace ducode {

class CellModule : public Cell {

public:
  CellModule(std::string name, std::string type, std::vector<ducode::Port>& ports, bool hidden, const nlohmann::json& parameters, const nlohmann::json& attributes)
      : Cell(std::move(name), std::move(type), ports, hidden, parameters, attributes) {}

  [[nodiscard]] bool is_module() const override {
    return true;
  }

  [[nodiscard]] std::string export_verilog(const nlohmann::json& params) const override {
    assert(params.contains("ift"));

    const bool ift = params["ift"].get<bool>();

    std::unordered_map<std::size_t, std::string> identifiers_map;
    if (params.contains("identifiers_map")) {
      identifiers_map = params["identifiers_map"].get<std::unordered_map<std::size_t, std::string>>();
    }

    std::stringstream result;

    std::vector<std::string> connections;
    for (const auto& port: m_ports) {
      if (identifiers_map.contains(port.m_bits_signature)) {
        std::string assignment = identifiers_map.at(port.m_bits_signature);

        connections.emplace_back(fmt::format(".{}({})", legalize_identifier(port.m_name), assignment));
        if (ift) {
          connections.emplace_back(fmt::format(".{}_t({}_t)", legalize_identifier(port.m_name), assignment));
        }
      }
    }

    auto connections_string = boost::algorithm::join(connections, ", ");
    result << fmt::format("  {} {}({});\n", legalize_identifier(m_type), legalize_identifier(m_name), connections_string);

    return result.str();
  }
};
}// namespace ducode