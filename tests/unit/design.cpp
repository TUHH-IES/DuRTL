/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/design.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <fmt/core.h>
#include <gsl/assert>
#include <nlohmann/json_fwd.hpp>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

/**
 \brief read in test for multiple designs
*/

TEST_CASE("JSON design parser", "[Design]") {
  //Read the json design into the data structure

  const auto* design_name = GENERATE("picorv32", "y86", "simple_spi_top", "mips_16_core_top");

  auto path = std::filesystem::path{TESTFILES_DIR} / design_name / fmt::format("{}.json", design_name);
  const auto design = ducode::Design::parse_json(path);

  //read the json file for comparison with the stored design_name
  std::ifstream json_stream(path);
  //requires file to be open
  Expects(json_stream.is_open());
  nlohmann::json json_file;
  json_stream >> json_file;

  REQUIRE(json_file.find("modules") != json_file.end());

  CHECK(design.get_modules().size() == json_file.find("modules")->size());
  for (const auto& mod: design.get_modules()) {
    CHECK(mod.get_cells().size() == json_file.find("modules")->find(mod.get_name())->find("cells")->size());
    CHECK(mod.get_ports().size() == json_file.find("modules")->find(mod.get_name())->find("ports")->size());
  }
}

TEST_CASE("check clock edge of all flipflops in top module", "[design]") {
  const std::string design_file = "dff_design";
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / design_file / (design_file + ".json");

  auto design = ducode::Design::parse_json(json_file);

  CHECK(design.check_clock_signal_consistency());
}
