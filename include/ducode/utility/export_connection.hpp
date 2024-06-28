//
// Created by gianluca on 01.02.24.
//

#pragma once

#include <range/v3/view/enumerate.hpp>

#include <string>

namespace ducode {

inline std::string export_connection(const std::string& source_name, const bit_container& source_bits, const std::string& target_name, const bit_container& target_bits) {
  std::stringstream result;

  if (source_bits == target_bits) {
    result << fmt::format("  assign {} = {};\n", target_name, source_name);
  } else {
    for (const auto& [target_index, target_bit]: ranges::views::enumerate(target_bits)) {

      if (!target_bit.is_constant()) {
        const std::string assignment_string = target_bits.size() > 1 ? fmt::format("{}[{}]", target_name, target_index) : target_name;

        for (const auto& [source_index, source_bit]: ranges::views::enumerate(source_bits)) {
          if (target_bit == source_bit) {
            if (source_bits.size() == 1) {
              result << fmt::format("  assign {} = {};\n", assignment_string, source_name);
            } else {
              result << fmt::format("  assign {} = {}[{}];\n", assignment_string, source_name, source_index);
            }
          }
        }
      }
    }
  }

  return result.str();
}

}// namespace ducode