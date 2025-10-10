/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/port.hpp>
#include <ducode/utility/legalize_identifier.hpp>

#include <boost/functional/hash.hpp>
#include <fmt/core.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <z3++.h>

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ducode {
class Cell {
protected:
  //NOLINTBEGIN(*-non-private-member-variables-in-classes)
  std::string m_name;
  std::string m_type;
  bool m_hidden = false;
  bool m_register = false;
  std::vector<ducode::Port> m_ports;
  nlohmann::json m_parameters;
  nlohmann::json m_attributes;
  //NOLINTEND(*-non-private-member-variables-in-classes)

public:
  Cell(std::string name, std::string type, std::vector<ducode::Port>& ports, bool hidden, nlohmann::json parameters, nlohmann::json attributes)
      : m_name(std::move(name)),
        m_type(std::move(type)),
        m_hidden(hidden),
        m_ports(ports),
        m_parameters(std::move(parameters)),
        m_attributes(std::move(attributes))

  {
    for (auto& port: m_ports) {
      port.m_identifier = fmt::format("{}_{}_port", legalize_identifier(m_name), port.m_name);
    }
  };

  Cell(const Cell& other) = default;
  Cell& operator=(const Cell& other) = default;
  Cell(Cell&& other) = default;
  Cell& operator=(Cell&& other) = default;
  virtual ~Cell() = default;

  [[nodiscard]] const std::vector<ducode::Port>& get_ports() const {
    return m_ports;
  }

  [[nodiscard]] const ducode::Port& get_port(std::string port_name) const {
    auto iter = std::ranges::find_if(m_ports, [&](const ducode::Port& port) { return port.m_name == port_name; });
    if (iter == m_ports.end()) {
      throw std::runtime_error(fmt::format("Port {} not found in cell {}", port_name, m_name));
    }
    return *iter;
  }

  [[nodiscard]] bool has_port(std::string port_name) const {
    return std::ranges::any_of(m_ports, [&](const ducode::Port& port) { return port.m_name == port_name; });
  }

  [[nodiscard]] std::string get_name() const {
    return m_name;
  };

  [[nodiscard]] std::string get_source_info() const {
    if (m_attributes.contains("src")) {
      return fmt::format("{}", m_attributes["src"].get<std::string>());
    }
    return fmt::format("src attribute not available.");
  };

  [[nodiscard]] const nlohmann::json& get_parameters() const {
    return m_parameters;
  };
  [[nodiscard]] const nlohmann::json& get_attributes() const {
    return m_attributes;
  };

  [[nodiscard]] bool is_hidden() const {
    return m_hidden;
  };

  [[nodiscard]] virtual bool is_module() const {
    return false;
  };

  [[nodiscard]] bool has_register() const {
    return m_register;
  };

  [[nodiscard]] virtual std::string export_verilog([[maybe_unused]] const nlohmann::json& params) const {
    throw std::runtime_error("Not implemented");
  }

  [[nodiscard]] const std::string& get_type() const {
    return m_type;
  }

  virtual void export_smt2([[maybe_unused]] z3::context& ctx, [[maybe_unused]] z3::solver& solver, [[maybe_unused]] const std::unordered_map<std::string, z3::expr>& port_expr_map, [[maybe_unused]] const nlohmann::json& params) const {
    throw std::runtime_error("Not implemented");
  };

  [[nodiscard]] virtual z3::expr get_cell_assertion_unary([[maybe_unused]] z3::context& ctx, [[maybe_unused]] const z3::expr& input1, [[maybe_unused]] const z3::expr& output) const {
    throw std::runtime_error("Not implemented");
  }
  [[nodiscard]] virtual z3::expr get_cell_assertion_binary([[maybe_unused]] z3::context& ctx, [[maybe_unused]] const z3::expr& input1, [[maybe_unused]] const z3::expr& input2, [[maybe_unused]] const z3::expr& output) const {
    throw std::runtime_error("Not implemented");
  }
};
}// namespace ducode
