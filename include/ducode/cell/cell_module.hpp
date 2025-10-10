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
          connections.emplace_back(fmt::format(".{}{}({}{})", legalize_identifier(port.m_name), ift_tag, assignment, ift_tag));
        }
      }
    }

    auto connections_string = boost::algorithm::join(connections, ", ");
    result << fmt::format("  {} {}({});\n", legalize_identifier(m_type), legalize_identifier(m_name), connections_string);

    return result.str();
  }


  void export_smt2([[maybe_unused]] z3::context& ctx, [[maybe_unused]] z3::solver& solver, const std::unordered_map<std::string, z3::expr>& port_expr_map, [[maybe_unused]] const nlohmann::json& params) const override {
    if (port_expr_map.size() != 2) {
      throw std::runtime_error(fmt::format(
          "Port expression map size is {}, but must be exactly 2.", port_expr_map.size()));
    }
    std::string port_name = params["port"];

    if (port_expr_map.at(fmt::format("{}_{}", port_name, "successor")).to_string() != port_expr_map.at(port_name).to_string()) {
      solver.add(port_expr_map.at(fmt::format("{}_{}", port_name, "successor")) == port_expr_map.at(port_name));
    }
  }
};
}// namespace ducode