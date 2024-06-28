/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/design.hpp>

#include <boost/filesystem.hpp>
#include <catch2/catch_all.hpp>

TEST_CASE("module graphs and split design", "[experiments]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "export" / "split" / "split.json";

  const auto design = ducode::Design::parse_json(json_file.string());

  for (const auto& module: design.get_modules()) {
    CHECK(module.get_graph().m_vertices.size() == 11);
  }
}

TEST_CASE("module graphs and hierarchy design", "[experiments]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "hierarchy" / "hierarchy.json";

  const auto design = ducode::Design::parse_json(json_file.string());

  for (const auto& module: design.get_modules()) {
    module.write_graphviz(fmt::format("{}.dot", module.get_name()));

    CHECK(module.get_graph().m_vertices.size() == (module.get_cells().size() + module.get_nets().size()));
  }
}