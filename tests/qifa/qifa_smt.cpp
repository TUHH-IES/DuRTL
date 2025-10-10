#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/qifa_smt.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <fmt/core.h>
#include <nlohmann/json_fwd.hpp>
#include <spdlog/spdlog.h>

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

using namespace ducode;


TEST_CASE("qifa_smt", "[qifa_smt]") {
  nlohmann::json params;
  params["smt_debug"] = true;
  params["clock"] = "clk";
  params["reset"] = "rst";
  params["secrets"] = std::vector<std::string>{"secret"};
  params["unroll_factor"] = static_cast<uint32_t>(3);
  params["intermediate_step"] = true;

  auto testcases = GENERATE(table<char const *, uint32_t>({
      {"qifa_exampleCombinational", 2},
      //{"qifa_exampleSplitting", 6},
      //{"qifa_exampleComplex2", 70},
      {"qifa_exampleTrivial", 4},
      {"qifa_exampleSimple", 2},
      //{"qifa_exampleComplex", 2}
  }));
  // read design
  char const *designName = std::get<0>(testcases);
  uint32_t expectedEquivalenceClasses = std::get<1>(testcases);

  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "qifa_example" / fmt::format("{}.json", designName);
  spdlog::info("Considering example: {}", json_file.string());

  /*
  json_file = boost::filesystem::path{TESTFILES_DIR} / "qifa_example" / "qifa_exampleComplex2.json";
  json_file = boost::filesystem::path{TESTFILES_DIR} / "qifa_example" / "qifa_exampleTrivial.json";
  json_file = boost::filesystem::path{TESTFILES_DIR} / "qifa_example" / "qifa_exampleSimple.json";
  json_file = boost::filesystem::path{TESTFILES_DIR} / "qifa_example" / "qifa_exampleComplex.json";
  json_file = boost::filesystem::path{TESTFILES_DIR} / "qifa_example" / "qifa_exampleSplitting.json";
  json_file = boost::filesystem::path{TESTFILES_DIR} / "qifa_example" / "qifa_exampleCombinational.json";
*/

  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);
  // read in json netlist
  auto design = Design::parse_json(json_file);

  uint32_t numMaxEquivalenceClasses = expectedEquivalenceClasses + 2;
  QifaSmt qifa(design, params);
  auto instTimeIOvals = qifa.find_equivalence_classes(numMaxEquivalenceClasses, params);

  CHECK(instTimeIOvals.size() == expectedEquivalenceClasses);
}
