/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ankerl/unordered_dense.h>

#include <unordered_set>
#include <vector>

namespace ducode {

/**
 * @brief Alias for a fast bit container using std::unordered_set.
 */
using fast_bit_container = std::unordered_set<Bit>;

/**
 * @brief Alias for a bit container using std::vector.
 */
using bit_container = std::vector<Bit>;

}// namespace ducode
