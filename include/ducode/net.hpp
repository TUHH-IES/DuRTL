/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/bit.hpp>
#include <ducode/ids.hpp>
#include <ducode/types.hpp>
#include <ducode/utility/legalize_identifier.hpp>

#include <boost/algorithm/string/join.hpp>
#include <boost/functional/hash.hpp>
#include <fmt/format.h>
#include <gsl/assert>
#include <nlohmann/json.hpp>
#include <range/v3/view/enumerate.hpp>

#include <limits>
#include <memory>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace ducode {

/**
 * @class Net
 * @brief Represents a network in a circuit.
 *
 * The `Net` class encapsulates the properties and behaviors of a network in a circuit.
 * It stores information such as the name, source information, bits, and initialization value of the network.
 * It also provides methods to access and manipulate these properties.
 */
class Net {

  std::string m_name;
  std::string m_src_name;
  uint32_t m_src_line = 0;
  std::size_t m_bits_signature;
  bit_container m_bits;
  fast_bit_container m_fast_search_bits;
  bool m_hidden = false;
  std::string m_init_value;
  bool m_contains_constant = false;
  bool m_contains_non_constant = false;

public:
  /**
   * @brief Constructs a Net object.
   *
   * This constructor initializes a Net object with the given parameters.
   *
   * @param net_name The name of the net.
   * @param hidden A boolean indicating whether the net is hidden.
   * @param bits A container of bits associated with the net.
   */
  Net(std::string net_name, bool hidden, bit_container bits)
      : m_name(std::move(net_name)),
        m_bits(std::move(bits)),
        m_fast_search_bits(m_bits.begin(), m_bits.end()),
        m_hidden(hidden),
        m_contains_constant(std::ranges::any_of(m_bits, [](const auto& bit) { return bit.is_constant(); })),
        m_contains_non_constant(std::ranges::any_of(m_bits, [](const auto& bit) { return !bit.is_constant(); })) {
    m_bits_signature = boost::hash_value(m_bits);
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
   * This function returns true if the object does not contain any non-constant elements,
   * and false otherwise.
   * 
   * @return True if the object contains only constants, false otherwise.
   */
  [[nodiscard]] bool contains_only_constants() const { return !m_contains_non_constant; }

  /**
   * @brief Checks if the object is hidden.
   * 
   * @return true if the object is hidden, false otherwise.
   */
  [[nodiscard]] bool is_hidden() const {
    return m_hidden;
  }

  /**
   * @brief Returns the number of bits in the signature.
   *
   * This function returns the number of bits in the signature.
   *
   * @return The number of bits in the signature.
   */
  [[nodiscard]] std::size_t get_bits_signature() const {
    return m_bits_signature;
  }

  /**
   * @brief Retrieves the bits stored in the bit container.
   * 
   * @return A reference to the const bit_container object containing the bits.
   */
  [[nodiscard]] const bit_container& get_bits() const {
    return m_bits;
  }

  /**
   * @brief Returns the fast search bits container.
   * 
   * This function returns a const reference to the fast search bits container.
   * 
   * @return const fast_bit_container& - A const reference to the fast search bits container.
   */
  [[nodiscard]] const fast_bit_container& get_fast_search_bits() const {
    return m_fast_search_bits;
  }

  /**
   * @brief Get the name of the object.
   * 
   * @return The name of the object as a string.
   */
  [[nodiscard]] std::string get_name() const {
    return m_name;
  }

  /**
   * Sets the initial value for the object.
   * 
   * @param init The initial value to be set.
   */
  void set_init_value(const std::string& init) {
    m_init_value = init;
  }

  /**
   * @brief Retrieves the initial value.
   * 
   * This function returns the initial value stored in the object.
   * 
   * @return The initial value as a string.
   */
  [[nodiscard]] std::string get_init_value() const {
    return m_init_value;
  }

  /**
   * Sets the source information for the object.
   *
   * @param src The name of the source file.
   * @param line The line number in the source file.
   */
  void set_source_info(const std::string& src, uint32_t line) {
    m_src_name = src;
    m_src_line = line;
  }

  /**
   * Returns the source information of the object.
   *
   * This function returns a string containing the source name and line number
   * of the object.
   *
   * @return A string containing the source information.
   */
  [[nodiscard]] std::string get_source_info() const {
    return fmt::format("{}:{}", m_src_name, m_src_line);
  }

  /**
   * @brief Exports the Verilog representation of the net.
   *
   * This function exports the Verilog representation of the net based on the provided parameters.
   *
   * @param params The parameters for exporting the Verilog representation.
   * @return The Verilog representation of the net as a string.
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

    const std::string legalized_name = legalize_identifier(m_name);

    result << fmt::format("  {} {} {};\n", net_type, net_width_string, legalized_name);
    if (ift) {
      result << fmt::format("  {} [{}:0] {}_t{};\n", net_type, tag_size - 1, legalized_name, is_register ? " = 0" : "");
    }

    if (is_register && !get_init_value().empty()) {
      result << fmt::format("initial begin\n");
      result << fmt::format("  {} = {};\n", legalized_name, get_init_value());
      result << fmt::format("end\n");
    }

    if (contains_constant()) {
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
      if (!contains_non_constant()) {
        const std::string assignment_string = fmt::format("{}", legalized_name + "_t");
        result << fmt::format("  assign {} = 0;\n", assignment_string);
      }
    }

    return result.str();
  }
};

}// namespace ducode