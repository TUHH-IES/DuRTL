/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/tag_generator.hpp>
#include <ducode/testbench.hpp>
#include <ducode/utility/types.hpp>

#include <boost/filesystem/operations.hpp>
#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json_fwd.hpp>
#include <spdlog/spdlog.h>
#include <vcd-parser/VCDTypes.hpp>

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

TEST_CASE("export tags", "[tagging]") {
  nlohmann::json params;
  params["ift"] = true;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;

  auto json_file = std::filesystem::path(TESTFILES_DIR) / "ift" / "tag_injection_test" / "ift_tag_test.json";
  auto reference_input_tags = boost::filesystem::path{TESTFILES_DIR} / "ift" / "tag_injection_test" / "ift_tag_test.txt";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  auto exported_verilog = temp_dir / "ift.v";
  auto exported_testbench = temp_dir / "ift_tb.v";
  auto exported_tags = temp_dir / "vcd_input.txt";

  spdlog::info(exported_tags.string());

  const auto des = ducode::Design::parse_json(json_file);
  const std::string testType = {"ift_tag_test"};
  const auto& mod = des.get_module(des.get_top_module_name());

  std::vector<VCDTime> timesteps = {0, 1, 2};
  params["first_simulation"] = true;
  ducode::Testbench testbench(des, nullptr);
  testbench.set_tag_generator(std::make_unique<ducode::FullResolutionTagGenerator>(mod.get_input_ports(), nullptr));
  std::vector<ExportedTagsFile> exported_tags_ = {ExportedTagsFile{.time_points = timesteps, .exported_tags_file_path = exported_tags}};

  testbench.write_tags(exported_tags_, params);

  auto txtFile = temp_dir / (testType + "VcdInput.txt");

  std::ifstream File1;
  std::ifstream File2;
  std::string line;
  std::string line2;

  File1.open(reference_input_tags.string());
  File2.open(exported_tags.string());
  assert(!File1.fail());
  assert(!File2.fail());
  bool result = true;
  while (!File1.eof()) {//read file until you reach the end
    getline(File1, line);
    getline(File2, line2);
    if (line != line2) {
      result = false;
    }

    File1.close();
    File2.close();
  }
  CHECK(result);
}