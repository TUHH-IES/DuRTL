//
// Created by Gianluca on 22/02/2023.
// Copyright (c) 2023 TUHH. All rights reserved.
//

#pragma once

#include <string>

[[nodiscard]] inline std::string replace_all_copy(const std::string_view str, const std::string_view source, const std::string_view target) {
  std::string result{str};
  if (source.empty()) {
    return result;
  }
  std::size_t start_pos = 0;
  while ((start_pos = result.find(source, start_pos)) != std::string::npos) {
    result.replace(start_pos, source.length(), target);
    start_pos += target.length();// In case 'target' contains 'source', like replacing 'x' with 'yx'
  }
  return result;
}

inline void replace_all(std::string& str, const std::string_view source, const std::string_view target) {
  if (source.empty()) {
    return;
  }
  std::size_t start_pos = 0;
  while ((start_pos = str.find(source, start_pos)) != std::string::npos) {
    str.replace(start_pos, source.length(), target);
    start_pos += target.length();// In case 'target' contains 'source', like replacing 'x' with 'yx'
  }
}
