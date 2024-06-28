//
// Created by gianluca on 19.10.23.
//

#pragma once

#include <ducode/utility/concepts.hpp>

namespace ducode {

template<range_like T, typename Element>
inline bool contains(const T& range, const Element& elem) {
  return std::ranges::find(range, elem) != range.end();
}

}// namespace ducode