/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/ids.hpp>
#include <ducode/port.hpp>
#include <ducode/utility/legalize_identifier.hpp>

#include <boost/functional/hash.hpp>
#include <boost/poly_collection/base_collection.hpp>
#include <fmt/core.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <limits>
#include <map>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <unordered_set>
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

  [[nodiscard]] std::string get_name() const {
    return m_name;
  };

  [[nodiscard]] std::string get_source_info() const {
    if (m_attributes.contains("src")) {
      return fmt::format("{}", m_attributes["src"].get<std::string>());
    }
    return fmt::format("src attribute not available.");
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
};
}// namespace ducode