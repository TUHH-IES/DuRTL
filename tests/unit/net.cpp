/* SPDX-License-Identifier: Apache-2.0 */

#include <ducode/net.hpp>

#include <catch2/catch_all.hpp>

TEST_CASE("net basic functionalities", "[net]") {
  const ducode::Net net("test", false, {2, 3, 4, 5});

  CHECK(net.get_name() == "test");
}