/* SPDX-License-Identifier: Apache-2.0 */

#include <ducode/utility/replace.hpp>

#include <catch2/catch_all.hpp>

TEST_CASE("replace_all", "[replace]") {
  std::string test_string = "replace\\test\\string";
  std::string reference_string = R"(replace\\test\\string)";

  replace_all(test_string, "", "\\\\");
  REQUIRE(test_string == "replace\\test\\string");

  replace_all(test_string, "\\", "\\\\");
  REQUIRE(test_string == reference_string);
}

TEST_CASE("replace_view", "[replace]") {
  const std::string test_string = "replace\\test\\string";
  std::string reference_string = R"(replace\\test\\string)";

  REQUIRE(replace_all_copy(std::string_view(test_string), "", "\\\\") == "replace\\test\\string");

  REQUIRE(replace_all_copy(std::string_view{test_string}, "\\", "\\\\") == reference_string);
}