//
// Created by Gianluca on 22/02/2023.
// Copyright (c) 2023 TUHH. All rights reserved.
//

#pragma once

#include <string>

[[nodiscard]] inline std::string replace_all_copy(const std::string_view str, const std::string_view from, const std::string_view to) {
  std::string result{str};
  if (from.empty()) {
    return result;
  }
  std::size_t start_pos = 0;
  while ((start_pos = result.find(from, start_pos)) != std::string::npos) {
    result.replace(start_pos, from.length(), to);
    start_pos += to.length();// In case 'to' contains 'from', like replacing 'x' with 'yx'
  }
  return result;
}

inline void replace_all(std::string& str, const std::string_view from, const std::string_view to) {
  if (from.empty()) {
    return;
  }
  std::size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length();// In case 'to' contains 'from', like replacing 'x' with 'yx'
  }
}