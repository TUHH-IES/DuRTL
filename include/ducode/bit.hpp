/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <boost/functional/hash.hpp>
#include <fmt/ostream.h>

#include <compare>
#include <limits>

namespace ducode {
/**
 * @brief Represents a bit value.
 */
class Bit {
public:
  inline static constexpr std::size_t const_bit_0 = 0UL;
  inline static constexpr std::size_t const_bit_1 = 1UL;
  inline static constexpr std::size_t const_bit_X = std::numeric_limits<std::size_t>::max();
  inline static constexpr std::size_t const_bit_Z = std::numeric_limits<std::size_t>::max() - 1;

  /**
   * @brief Default constructor.
   */
  Bit() = default;

  /**
   * @brief Constructs a Bit object with the given bit value.
   * @param bit The bit value.
   */
  Bit(std::size_t bit) : m_bit{bit} {}//NOLINT(google-explicit-constructor,hicpp-explicit-conversions)

  /**
   * @brief Compares two Bit objects for equality.
   * @param other The other Bit object to compare.
   * @return True if the Bit objects are equal, false otherwise.
   */
  auto operator<=>(const Bit& other) const = default;//NOLINT(*-use-nullptr)

  /**
   * @brief Compares a Bit object with a bit value for equality.
   * @param bit The bit value to compare.
   * @return The result of the comparison.
   */
  auto operator<=>(const std::size_t& bit) const {
    return bit <=> m_bit;
  }

  /**
   * @brief Checks if the Bit object represents a constant bit value.
   * @return True if the Bit object represents a constant bit value, false otherwise.
   */
  [[nodiscard]] bool is_constant() const {
    return m_bit == const_bit_0 ||
           m_bit == const_bit_1 ||
           m_bit == const_bit_X ||
           m_bit == const_bit_Z;
  }

private:
  std::size_t m_bit = const_bit_X;

public:
  /**
   * @brief Overloads the << operator to output the Bit object.
   * @param os The output stream.
   * @param bit The Bit object to output.
   * @return The output stream.
   */
  friend std::ostream& operator<<(std::ostream& os, const Bit& bit) {
    if (bit == const_bit_X) {
      return os << "X";
    }

    if (bit == const_bit_Z) {
      return os << "Z";
    }

    return os << bit.m_bit;
  }

  friend std::size_t hash_value(const ducode::Bit& bit);
};

/**
 * Calculates the hash value for a ducode::Bit object.
 *
 * @param bit The ducode::Bit object to calculate the hash value for.
 * @return The calculated hash value.
 */
inline std::size_t hash_value(const ducode::Bit& bit) {
  boost::hash<std::size_t> hasher;
  return hasher(bit.m_bit);
}

}// namespace ducode

template<>
struct fmt::formatter<ducode::Bit> : ostream_formatter {};

template<>
struct std::hash<ducode::Bit> {
  std::size_t operator()(const ducode::Bit& bit) const {
    boost::hash<ducode::Bit> bit_hasher;
    return bit_hasher(bit);
  }
};