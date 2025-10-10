/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/design.hpp>

#include <catch2/catch_test_macros.hpp>
//#include <fmt/core.h>

#include <string>

TEST_CASE("module graphs and split design", "[experiments]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "export" / "split" / "split.json";

  const auto design = ducode::Design::parse_json(json_file);

  for (const auto& module: design.get_modules()) {
    CHECK(module.get_graph().m_vertices.size() == 11);
  }
}

TEST_CASE("module graphs and hierarchy design", "[experiments]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "hierarchy" / "hierarchy.json";

  const auto design = ducode::Design::parse_json(json_file);

  for (const auto& module: design.get_modules()) {
    //module.write_graphviz(fmt::format("{}.dot", module.get_name()));

    CHECK(module.get_graph().m_vertices.size() == (module.get_cells().size() + module.get_nets().size()));
  }
}

TEST_CASE("check clock edge polarity of all flipflops in module", "[design]") {
  const std::string design_file = "dff_design";
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / design_file / (design_file + ".json");

  auto design = ducode::Design::parse_json(json_file);

  CHECK(design.get_module("dff_design").check_clock_rising_edge_consistency());
}

TEST_CASE("check clock edge polarity of all flipflops in module diff edges", "[design]") {
  const std::string design_file = "dff_design";
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / (design_file + "_posneg_edge") / (design_file + ".json");

  auto design = ducode::Design::parse_json(json_file);

  CHECK(!design.get_module("dff_design").check_clock_rising_edge_consistency());
}
