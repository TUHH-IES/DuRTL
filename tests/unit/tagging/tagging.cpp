/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/ift_analysis.hpp>
#include <ducode/signals_data_manager.hpp>
#include <ducode/tag_generator.hpp>
#include <ducode/testbench.hpp>
#include <ducode/utility/VCD_utility.hpp>
#include <ducode/utility/iverilog_wrapper.hpp>
#include <ducode/utility/simulation.hpp>

#include "ducode/instantiation_graph_traits.hpp"
#include "ducode/utility/types.hpp"
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <catch2/catch_test_macros.hpp>
#include <gsl/narrow>
#include <nlohmann/json_fwd.hpp>
#include <spdlog/spdlog.h>
#include <vcd-parser/VCDFile.hpp>
#include <vcd-parser/VCDTimedValue.hpp>
#include <vcd-parser/VCDTypes.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

TEST_CASE("tagging_inputs_multi_timestep", "[tagging_tests]") {
  nlohmann::json params;

  const std::vector<std::string> designs = {"and"};

  nlohmann::json result_data_full;

  params["ift"] = false;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;

  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / "basic_gates" / "binary_gates" / designs[0] / (designs[0] + "_tb.v");
  auto json_file = std::filesystem::path(TESTFILES_DIR) / "basic_gates" / "binary_gates" / designs[0] / (designs[0] + ".json");

  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  auto exported_ift_verilog = temp_dir / ("ift" + designs[0] + ".v");
  auto exported_verilog = temp_dir / (designs[0] + ".v");
  auto exported_testbench = temp_dir / ("ift" + designs[0] + "_tb.v");

  auto design = ducode::Design::parse_json(json_file);
  design.write_verilog(exported_verilog, params);

  // original design Simulation
  spdlog::info("Simulating Original Design + Testbench");
  auto vcd_data = ducode::simulate_design(exported_verilog, testbench_file);
  spdlog::info("End of Simulation");

  params["ift"] = true;
  design.write_verilog(exported_ift_verilog, params);

  //get top module
  auto top_module = design.get_top_module();
  const int stepsize = 1;
  auto timesteps_per_simulation_run = tag_size / top_module.get_input_names().size();
  timesteps_per_simulation_run = std::max<size_t>(timesteps_per_simulation_run, 1);
  auto number_simulation_runs = ducode::get_number_of_simulation_runs(stepsize, timesteps_per_simulation_run, vcd_data);
  params["timesteps_per_simulation_run"] = timesteps_per_simulation_run;
  //exporting testbench with IFT
  const VCDScope *root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));
  ;
  testbench.set_tag_generator(
      std::make_unique<ducode::FullResolutionTagGenerator>(top_module.get_input_ports(), vcd_data));
  testbench.write_verilog(exported_testbench, params);

  std::vector<boost::filesystem::path> exported_tags_files;
  for (uint32_t i = 0; i < number_simulation_runs; ++i) {
    boost::filesystem::path tag_file = temp_dir / ("vcd_input_" + std::to_string(i) + ".txt");
    exported_tags_files.push_back(tag_file);
  }
  std::vector<ExportedTagsFile> exported_tags_vec;
  std::vector<VCDTime> tags_times;
  for (auto time = 0u; time <= gsl::narrow<std::size_t>(vcd_data->get_timestamps().back()); time += stepsize) {
    tags_times.emplace_back(time);
  }
  /// here add loop which runs the simulation again for all created tag_map files.
  for (uint32_t i = 0; i < number_simulation_runs; ++i) {
    // For the ith batch of timestamps
    uint32_t items_per_batch = timesteps_per_simulation_run;
    uint32_t start_idx = i * items_per_batch;
    uint32_t end_idx = std::min(((i + 1) * items_per_batch), gsl::narrow<uint32_t>(tags_times.size()));

    std::vector<VCDTime> batch(tags_times.begin() + start_idx,
                               tags_times.begin() + end_idx);

    exported_tags_vec.emplace_back(batch, exported_tags_files[i]);
  }
  testbench.write_tags(exported_tags_vec, params);/// IFT simulation tag_map[0]
  spdlog::info("Simulating Exported Design + Testbench");
  auto data = ducode::simulate_design(exported_ift_verilog, exported_testbench, exported_tags_files[0]);
  spdlog::info("End of Simulation");

  // checking number of entries into the tag vectors
  CHECK(data->get_signal_values(std::ranges::find_if(data->get_signals(), [&](const auto &sig) { return (sig.reference == "a_t"); })->hash).size() == 5);
  CHECK(data->get_signal_values(std::ranges::find_if(data->get_signals(), [&](const auto &sig) { return (sig.reference == "b_t"); })->hash).size() == 5);
  CHECK(data->get_signal_values(std::ranges::find_if(data->get_signals(), [&](const auto &sig) { return (sig.reference == "c_t"); })->hash).size() == 5);

  //  CHECK(instance.m_graph.m_edges.size() == 3);
  //  CHECK(instance.m_graph.m_edges.front().get_property().name == "a");
  //  CHECK(instance.m_graph.m_edges.front().get_property().tags.size() == 5);
  //  CHECK(instance.m_graph.m_edges.front().get_property().values.size() == 4);
}

namespace {
inline uint32_t value_vector_to_uint32(const VCDBitVector &value_vector) {
  uint32_t result = 0;
  for (uint64_t i = 0; i < value_vector.size(); ++i) {
    if (value_vector[i] == VCDBit::VCD_1) {
      result |= (1u << i);
    }
  }
  return result;
}

inline uint32_t value_vector_to_uint32(const ducode::SignalBitVector &value_vector) {
  uint32_t result = 0;
  for (uint64_t i = 0; i < value_vector.size(); ++i) {
    if (value_vector[i] == ducode::SignalBit::BIT_1) {
      result |= (1u << i);
    }
  }
  return result;
}
}// namespace

TEST_CASE("value_vector_to_uint32 conversion", "[ift_analysis]") {
  SECTION("Empty vector should return 0") {
    VCDBitVector empty_vector;
    REQUIRE(ducode::value_vector_to_uint32(empty_vector) == 0);
  }

  SECTION("Single bit VCD_1 should return 1") {
    VCDBitVector single_bit;
    single_bit.push_back(VCDBit::VCD_1);
    REQUIRE(ducode::value_vector_to_uint32(single_bit) == 1);
  }

  SECTION("Single bit VCD_0 should return 0") {
    VCDBitVector single_bit;
    single_bit.push_back(VCDBit::VCD_0);
    REQUIRE(ducode::value_vector_to_uint32(single_bit) == 0);
  }

  SECTION("4-bit value 1010 should return 10") {
    VCDBitVector bits;
    bits.push_back(VCDBit::VCD_1);
    bits.push_back(VCDBit::VCD_0);
    bits.push_back(VCDBit::VCD_1);
    bits.push_back(VCDBit::VCD_0);
    REQUIRE(ducode::value_vector_to_uint32(bits) == 10);
  }

  SECTION("8-bit value 11111111 should return 255") {
    VCDBitVector bits;
    const uint32_t bits_number = 8;
    for (int i = 0; i < bits_number; i++) {
      bits.push_back(VCDBit::VCD_1);
    }
    REQUIRE(ducode::value_vector_to_uint32(bits) == 255);
  }

  SECTION("VCD_X value should throw exception") {
    VCDBitVector bits;
    bits.push_back(VCDBit::VCD_1);
    bits.push_back(VCDBit::VCD_X);// This should cause an exception
    REQUIRE_THROWS_AS(ducode::value_vector_to_uint32(bits), std::runtime_error);
  }

  SECTION("VCD_Z value should throw exception") {
    VCDBitVector bits;
    bits.push_back(VCDBit::VCD_1);
    bits.push_back(VCDBit::VCD_Z);// This should cause an exception
    REQUIRE_THROWS_AS(ducode::value_vector_to_uint32(bits), std::runtime_error);
  }

  SECTION("32-bit maximum value") {
    VCDBitVector bits;
    const uint32_t bits_number = 32;
    for (int i = 0; i < bits_number; i++) {
      bits.push_back(VCDBit::VCD_1);
    }
    REQUIRE(ducode::value_vector_to_uint32(bits) == UINT32_MAX);
  }

  SECTION("VCDBitVector input is 32 bit or less") {
    VCDBitVector bits;
    const uint32_t bits_number = 33;
    for (int i = 0; i < bits_number; i++) {
      bits.push_back(VCDBit::VCD_1);
    }
    REQUIRE_THROWS_AS(ducode::value_vector_to_uint32(bits), std::runtime_error);
  }

  SECTION("Bit order verification") {
    VCDBitVector bits;
    const uint32_t bits_number = 7;
    // Create 0000...0001 (only LSB is set)
    for (int i = 0; i < bits_number; i++) {
      bits.push_back(VCDBit::VCD_0);
    }
    bits.push_back(VCDBit::VCD_1);
    // This should be 1, not 128 (which would happen if bit order was wrong)
    REQUIRE(ducode::value_vector_to_uint32(bits) == 1);
  }
}


TEST_CASE("tag_decoding", "[tagging]") {
  nlohmann::json params;
  params["ift"] = true;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;

  auto verilog_file = boost::filesystem::path(TESTFILES_DIR) / "ift_flow" / "hierarchy.v";
  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / "ift_flow" / "hierarchy_ift_flow_tb.v";
  auto json_file = boost::filesystem::path(TESTFILES_DIR) / "ift_flow" / "hierarchy.json";
  auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / "ift_flow" / "hierarchy_ift";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  auto design = ducode::Design::parse_json(json_file);

  auto vcd_data = ducode::simulate_design(verilog_file, testbench_file);

  auto exported_verilog = temp_dir / "ift_design.v";
  auto exported_testbench = temp_dir / "ift_testbench.v";

  design.write_verilog(exported_verilog, params);

  auto top_module = design.get_top_module();
  const int stepsize = 8;

  auto timesteps_per_simulation_run = get_timesteps_per_simulation_run(top_module, tag_size);
  params["timesteps_per_simulation_run"] = timesteps_per_simulation_run;

  auto number_simulation_runs = ducode::get_number_of_simulation_runs(stepsize, timesteps_per_simulation_run, vcd_data);

  spdlog::info("Running {} simulations", number_simulation_runs);

  std::vector<boost::filesystem::path> exported_tags_files = ducode::get_exported_tags_files(number_simulation_runs);

  const VCDScope *root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager signal_tracker(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(signal_tracker));
  testbench.set_tag_generator(std::make_unique<ducode::FullResolutionTagGenerator>(top_module.get_input_ports(), vcd_data));

  std::vector<VCDTime> tags_times = ducode::get_tags_times(vcd_data, stepsize);

  testbench.write_verilog(exported_testbench, params);
  auto exported_tags_vec = ducode::get_exported_tags_vec(exported_tags_files, number_simulation_runs, tags_times, timesteps_per_simulation_run);

  testbench.write_tags(exported_tags_vec, params);

  spdlog::info("Simulating Exported Design + Testbench");


  std::vector<std::shared_ptr<VCDFile>> data_vec;
  for (const auto &exported_tags: exported_tags_files) {
    auto data = ducode::simulate_design(exported_verilog, exported_testbench, exported_tags);
    data_vec.emplace_back(data);
  }
  std::vector<const VCDScope *> vcd_root_scopes;
  vcd_root_scopes.reserve(data_vec.size());
  for (auto &vcd_file: data_vec) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager sim_data(data_vec, vcd_root_scopes, data_vec.size());

  auto input_names = design.get_top_module().get_input_names();
  std::vector<std::string> input_hierarchy_vec = {"hierarchy"};
  std::vector<ducode::SignalIdentifier> input_ids;
  input_ids.reserve(input_names.size());
  for (auto &name: input_names) {
    input_ids.push_back({.name = name, .hierarchy = input_hierarchy_vec});
  }

  std::string target_signal_name = "c";
  std::vector<std::string> hierarchy_vec = {"hierarchy", "m1", "s1"};
  // instance.new_function(input_ids, output_id, target_timestep);
  ducode::TagSourceList tag_list = sim_data.get_information_flow_source_signals_for_target_signal_at_timestep(input_ids, {.name = target_signal_name, .hierarchy = hierarchy_vec}, 0);
  REQUIRE(tag_list.m_tag_vector.size() == 1);
  CHECK(tag_list.m_tag_vector.at(0).input_id.name == "b");
  CHECK(tag_list.m_tag_vector.at(0).tag_injection_time == 0);

  const uint32_t timestep_2 = 8;
  tag_list = sim_data.get_information_flow_source_signals_for_target_signal_at_timestep(input_ids, {.name = target_signal_name, .hierarchy = hierarchy_vec}, timestep_2);
  REQUIRE(tag_list.m_tag_vector.size() == 2);
  CHECK(tag_list.m_tag_vector.at(0).input_id.name == "b");
  CHECK(tag_list.m_tag_vector.at(0).tag_injection_time == 8);
  CHECK(tag_list.m_tag_vector.at(1).input_id.name == "a");
  CHECK(tag_list.m_tag_vector.at(1).tag_injection_time == 8);
}
