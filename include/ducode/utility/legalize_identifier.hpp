//
// Created by gianluca on 18.04.23.
//

#pragma once

#include <ducode/utility/concepts.hpp>
#include <ducode/utility/replace.hpp>

#include <string>

template<string_like T>
[[nodiscard]] inline std::string legalize_identifier(const T& identifier) {
  std::string local = {identifier};
  replace_all(local, "$", "_");
  replace_all(local, ".", "_");
  replace_all(local, ":", "_");
  replace_all(local, "/", "_");
  replace_all(local, "\\", "_");
  replace_all(local, "[", "_");
  replace_all(local, "]", "_");
  replace_all(local, "'", "_");
  replace_all(local, "=", "_");
  return local;
}