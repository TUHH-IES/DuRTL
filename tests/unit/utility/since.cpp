/* SPDX-License-Identifier: Apache-2.0 */

#include <ducode/utility/since.hpp>

#include <catch2/catch_test_macros.hpp>

#include <chrono>

TEST_CASE("since", "[since]") {
  auto start = std::chrono::steady_clock::now();
  REQUIRE(since(start).count() == 0);
}