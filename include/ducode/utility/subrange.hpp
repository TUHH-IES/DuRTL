//
// Created by gianluca on 14.02.24.
//

#pragma once

#include <ranges>

namespace ducode {

template<typename T>
inline auto subrange(std::pair<T, T> iterators) {
  return std::ranges::subrange(iterators.first, iterators.second);
}

}// namespace ducode