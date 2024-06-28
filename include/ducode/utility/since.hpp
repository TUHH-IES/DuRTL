//
// Created by Gianluca on 09/02/2023.
// Copyright (c) 2023 TUHH. All rights reserved.
//

#pragma once

#include <chrono>

template<
    class result_t = std::chrono::milliseconds,
    class clock_t = std::chrono::steady_clock,
    class duration_t = std::chrono::milliseconds>
auto since(std::chrono::time_point<clock_t, duration_t> const& start) {
  return std::chrono::duration_cast<result_t>(clock_t::now() - start);
}