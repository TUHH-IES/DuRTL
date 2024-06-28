/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/generate_tags.hpp>
#include <ducode/utility/iverilog_wrapper.hpp>

#include <boost/filesystem.hpp>
#include <catch2/catch_all.hpp>
#include <spdlog/spdlog.h>

#include <string>


TEST_CASE("export tags", "[tagging]") {
  nlohmann::json params;
  params["ift"] = true;

  auto json_file = std::filesystem::path(TESTFILES_DIR) / "ift" / "tag_injection_test" / "ift_tag_test.json";
  auto reference_input_tags = boost::filesystem::path{TESTFILES_DIR} / "ift" / "tag_injection_test" / "ift_tag_test.txt";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  auto exported_verilog = temp_dir / "ift.v";
  auto exported_testbench = temp_dir / "ift_tb.v";
  auto exported_tags = temp_dir / "vcd_input.txt";

  spdlog::info(exported_tags.string());

  const auto des = ducode::Design::parse_json(json_file.string());
  auto path = std::filesystem::path(TESTFILES_DIR) / "ift";
  const std::string testType = {"ift_tag_test"};
  const auto& mod = des.get_module(des.get_top_module_name());

  auto tag_map = generate_tags_full_resolution(2, 1, mod.get_ports());
  auto tag_map_sliced = tag_map_splitting(&mod, tag_map);

  for (auto& slice: tag_map_sliced) {
    std::map<uint32_t, std::pair<uint32_t, ducode::Port>> encoded_tags = encode_tags(slice);

    export_tags(&mod, exported_tags.string(), encoded_tags);
  }

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


TEST_CASE("export_flow", "[ift_test][.]") {
  nlohmann::json params;
  params["ift"] = true;

  auto json_file = std::filesystem::path(TESTFILES_DIR) / "ift" / "ift_tag_test.json";
  auto reference_input_tags = boost::filesystem::path{TESTFILES_DIR} / "ift" / "ift_tag_test.txt";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  auto exported_verilog = temp_dir / ("ift.v");
  auto exported_testbench = temp_dir / ("ift_tb.v");
  auto exported_tags = temp_dir / "vcd_input.txt";

  const auto des = ducode::Design::parse_json(json_file.string());
  auto path = std::filesystem::path(TESTFILES_DIR) / "ift";
  const std::string testType = {"ift_tag_test"};
  const auto mod = des.get_module(des.get_top_module_name());

  auto tag_map = generate_tags_full_resolution(2, 1, mod.get_ports());
  auto tag_map_sliced = tag_map_splitting(&mod, tag_map);

  for (auto& slice: tag_map_sliced) {
    std::map<uint32_t, std::pair<uint32_t, ducode::Port>> encoded_tags = encode_tags(slice);

    export_tags(&mod, exported_tags.string(), encoded_tags);
  }

  auto txtFile = temp_dir / (testType + "VcdInput.txt");


  std::ifstream File1;
  std::ifstream File2;
  std::string line;
  std::string line2;

  File1.open(reference_input_tags.string());
  File2.open(txtFile.string());
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