/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/design.hpp>

#include <catch2/catch_all.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <string>
#include <vector>

/**
 \brief read in test for multiple designs
*/

TEST_CASE("multiple_designs_parse", "[DesignParserTest][StableCode]") {
  //Read the json design into the data structure

  const std::vector<std::string> design_names = {"picorv32_2", "moduleinstanceTest", "y86", "simple_spi", "mips_16"};

  for (const auto& design_name: design_names) {

    auto path = std::filesystem::path{TESTFILES_DIR} / "read_in" / (design_name + ".json");
    const auto design = ducode::Design::parse_json(path);

    //read the json file for comparison with the stored design_name
    std::ifstream json_stream(path);
    //requires file to be open
    Expects(json_stream.is_open());
    nlohmann::json json_file;
    json_stream >> json_file;

    REQUIRE(json_file.find("modules") != json_file.end());

    CHECK(design.get_filename() == path);
    CHECK(design.get_modules().size() == json_file.find("modules")->size());
    for (const auto& mod: design.get_modules()) {
      CHECK(mod.get_cells().size() == json_file.find("modules")->find(mod.get_name())->find("cells")->size());
      CHECK(mod.get_ports().size() == json_file.find("modules")->find(mod.get_name())->find("ports")->size());
    }
  }
}

TEST_CASE("multiple_designs_parse benchmark", "[DesignParserTest][.]") {
  auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("benchmark.log", true);
  file_sink->set_level(spdlog::level::trace);
  file_sink->set_pattern("%^[%l] %s:%# %! %v");
  spdlog::set_default_logger(std::make_shared<spdlog::logger>("benchmark_sink", spdlog::sinks_init_list({file_sink})));

  const std::vector<std::string> design_names = {
      //      "picorv32_2",
      //      "counter", "mux", "hierarchyVector", "moduleinstanceTest",
      "y86",
      //      "simple_spi", "mips_16", "split"
  };

  for (std::string design_name: design_names) {
    auto path = std::filesystem::path{TESTFILES_DIR} / "read_in" / (design_name + ".json");

    BENCHMARK(std::move(design_name)) {
      return ducode::Design::parse_json(path);
    };
  }
}