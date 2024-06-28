/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/net.hpp>
#include <ducode/types.hpp>

#include <fmt/ostream.h>
#include <range/v3/view/enumerate.hpp>

#include <algorithm>
#include <optional>
#include <string>
#include <vector>

namespace ducode {

/**
 * @brief Represents a port in a system.
 *
 * The `Port` class encapsulates the properties and behavior of a port, including its name, identifier, direction,
 * bit container, and whether it is signed or not. It provides methods to check if the port contains constants or
 * non-constants, and to export the port to Verilog code.
 */
class Port {
public:
  /**
   * @brief Enumeration representing the direction of a port.
   * 
   * This enumeration defines the possible directions of a port: input, output, inout, and unknown.
   * 
   * @note The underlying type of this enumeration is uint8_t.
   */
  enum class Direction : uint8_t {
    input,  /**< The port is an input port. */
    output, /**< The port is an output port. */
    inout,  /**< The port is an input/output port. */
    unknown /**< The direction of the port is unknown. */
  };

  //NOLINTBEGIN(*-non-private-member-variables-in-classes)
  std::string m_name;
  std::string m_identifier;
  Direction m_direction = Direction::unknown;
  bit_container m_bits;
  fast_bit_container m_fast_search_bits;
  std::size_t m_bits_signature = 0;
  bool m_signed = false;
  //NOLINTEND(*-non-private-member-variables-in-classes)

  Port() = default;

  Port(std::string name, std::string identifier, Direction direction, bit_container bits, bool sign)
      : m_name(std::move(name)),
        m_identifier(std::move(identifier)),
        m_direction(direction),
        m_bits(std::move(bits)),
        m_fast_search_bits(m_bits.begin(), m_bits.end()),
        m_bits_signature(boost::hash_value(m_bits)),
        m_signed(sign),
        m_contains_constant(std::ranges::any_of(m_bits, [](const auto& bit) { return bit.is_constant(); })),
        m_contains_non_constant(std::ranges::any_of(m_bits, [](const auto& bit) { return !bit.is_constant(); })) {
    m_fast_search_bits.erase(Bit::const_bit_X);
    m_fast_search_bits.erase(Bit::const_bit_Z);
    m_fast_search_bits.erase(Bit::const_bit_0);
    m_fast_search_bits.erase(Bit::const_bit_1);
  }

  /**
   * @brief Checks if the object contains a constant.
   *
   * @return true if the object contains a constant, false otherwise.
   */
  [[nodiscard]] bool contains_constant() const { return m_contains_constant; }
  /**
   * @brief Checks if the object contains a non-constant value.
   *
   * @return true if the object contains a non-constant value, false otherwise.
   */
  [[nodiscard]] bool contains_non_constant() const { return m_contains_non_constant; }
  /**
   * @brief Checks if the object contains only constants.
   * 
   * @return true if the object contains only constants, false otherwise.
   */
  [[nodiscard]] bool contains_only_constants() const { return !m_contains_non_constant; }

  /**
   * @brief Compares two Port objects for ordering.
   * 
   * This function compares the current Port object with the given `other` Port object
   * and returns the result of the comparison. The comparison is based on the values of
   * the `m_name`, `m_identifier`, `m_direction`, `m_bits`, `m_bits_signature`, and `m_signed`
   * member variables.
   * 
   * @param other The Port object to compare with.
   * @return A value indicating the ordering of the two Port objects.
   */
  auto operator<=>(const Port& other) const {
    return std::tie(m_name, m_identifier, m_direction, m_bits, m_bits_signature, m_signed) <=>
           std::tie(other.m_name, other.m_identifier, other.m_direction, other.m_bits, other.m_bits_signature, other.m_signed);
  }

  /**
   * @brief Overloaded equality operator for comparing two Port objects.
   * 
   * This operator compares the name, identifier, direction, bits, bits_signature, and signed
   * members of two Port objects for equality.
   * 
   * @param other The Port object to compare with.
   * @return true if the Port objects are equal, false otherwise.
   */
  bool operator==(const Port& other) const {
    return std::tie(m_name, m_identifier, m_direction, m_bits, m_bits_signature, m_signed) ==
           std::tie(other.m_name, other.m_identifier, other.m_direction, other.m_bits, other.m_bits_signature, other.m_signed);
  }

  /**
   * @brief Exports the Verilog representation of the port.
   *
   * This function exports the Verilog representation of the port based on the provided parameters.
   * It generates the Verilog code for the port declaration, including the net type, width, and name.
   * If the `ift` parameter is true, it also generates the code for the internal flip-flop register.
   * Additionally, if the port contains constant bits, it generates the code for assigning constant values.
   *
   * @param params The parameters for exporting the Verilog code.
   * @return The Verilog representation of the port.
   */
  [[nodiscard]] std::string export_verilog(const nlohmann::json& params) const {
    Expects(params.contains("ift"));
    const bool ift = params["ift"].get<bool>();

    bool is_register = false;
    if (params.contains("is_register")) {
      is_register = params["is_register"].get<bool>();
    }

    std::stringstream result;

    std::string net_type = "wire";
    if (is_register) {
      net_type = "reg";
    }

    std::string net_width_string;
    auto net_width = m_bits.size();
    if (net_width > 1) {
      net_width_string = fmt::format("[{}:0]", net_width - 1);
    }

    const std::string legalized_name = legalize_identifier(m_identifier);

    result << fmt::format("  {} {} {};\n", net_type, net_width_string, legalized_name);
    if (ift) {
      result << fmt::format("  {} [{}:0] {}_t{};\n", net_type, tag_size - 1, legalized_name, is_register ? " = 0" : "");
    }

    if (m_contains_constant) {
      for (const auto& [index, bit]: ranges::views::enumerate(m_bits)) {
        const std::string assignment_string = m_bits.size() > 1 ? fmt::format("{}[{}]", legalized_name, index) : legalized_name;
        if (bit == Bit::const_bit_Z) {
          // do nothing
        } else if (bit.is_constant()) {
          result << fmt::format("  assign {} = 1'b{};\n", assignment_string, bit);
        }
      }
    }

    if (ift) {
      if (!m_contains_non_constant) {
        const std::string assignment_string = fmt::format("{}", legalized_name + "_t");
        result << fmt::format("  assign {} = 0;\n", assignment_string);
      }
    }

    return result.str();
  }

private:
  bool m_contains_constant = false;
  bool m_contains_non_constant = false;
};

/**
 * Overloads the << operator to allow printing the Port::Direction enum values.
 * 
 * @param os The output stream to write to.
 * @param direction The Port::Direction value to be printed.
 * @return The modified output stream.
 * @throws std::runtime_error if the direction is not supported.
 */
inline std::ostream& operator<<(std::ostream& os, const Port::Direction& direction) {
  if (direction == Port::Direction::input) {
    return os << "input";
  }
  if (direction == Port::Direction::output) {
    return os << "output";
  }
  if (direction == Port::Direction::inout) {
    return os << "inout";
  }
  throw std::runtime_error("direction not supported");
}
}// namespace ducode

template<>
struct fmt::formatter<ducode::Port::Direction> : ostream_formatter {};

template<>
struct std::hash<ducode::Port> {
  std::size_t operator()(const ducode::Port& port) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, port.m_identifier);
    boost::hash_combine(seed, port.m_bits);
    return seed;
  }
};
