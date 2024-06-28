//
// Created by gianluca on 24.10.23.
//

#pragma once

#include <ducode/types.hpp>

#include <nlohmann/json.hpp>

inline ducode::bit_container to_bit_vector(const nlohmann::json& jbits) {
  Expects(jbits.is_array());

  ducode::bit_container duBits;
  duBits.reserve(jbits.size());

  for (const auto& jbit: jbits) {
    if (jbit.is_string()) {
      if (jbit == "0") {
        duBits.emplace_back(ducode::Bit::const_bit_0);
      } else if (jbit == "1") {
        duBits.emplace_back(ducode::Bit::const_bit_1);
      } else if (jbit == "x") {
        duBits.emplace_back(ducode::Bit::const_bit_X);
      } else if (jbit == "z") {
        duBits.emplace_back(ducode::Bit::const_bit_Z);
      } else {
        throw std::runtime_error(fmt::format("Unexpected value: {}", nlohmann::to_string(jbit)));
      }
    } else {
      duBits.emplace_back(jbit.get<std::size_t>());
    }
  }

  Ensures(duBits.size() == jbits.size());
  return duBits;
}
