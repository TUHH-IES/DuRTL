/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/ift_analysis.hpp>
#include <ducode/instantiation_graph.hpp>
#include <ducode/signals_data_manager.hpp>
#include <ducode/tag_generator.hpp>
#include <ducode/testbench.hpp>
#include <ducode/utility/VCD_utility.hpp>
#include <ducode/utility/iverilog_wrapper.hpp>
#include <ducode/utility/simulation.hpp>

#include "ducode/utility/types.hpp"
#include <boost/filesystem/operations.hpp>
#include <catch2/catch_test_macros.hpp>
#include <gsl/narrow>
#include <nlohmann/json_fwd.hpp>
#include <spdlog/spdlog.h>
#include <vcd-parser/VCDFile.hpp>
#include <vcd-parser/VCDFileParser.hpp>
#include <vcd-parser/VCDTypes.hpp>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

TEST_CASE("ift_analysis_test", "[ift_analysis_test]") {
  nlohmann::json params;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;

  std::pair<std::string, std::string> designs = {"analysis_test", "clk"};

  nlohmann::json result_data_full;
  params["ift"] = true;

  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / designs.first / (designs.first + "_tb.v");
  auto json_file = std::filesystem::path(TESTFILES_DIR) / designs.first / (designs.first + ".json");
  auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / designs.first / (designs.first + ".vcd");

  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  auto exported_verilog = temp_dir / ("ift" + designs.first + ".v");
  auto exported_testbench = temp_dir / ("ift" + designs.first + "_tb.v");

  ducode::Design design = ducode::Design::parse_json(json_file);

  ducode::DesignInstance instance = ducode::DesignInstance::create_instance(design);
  //instance.write_graphviz("ift_analysis_test.dot");

  //get top module
  auto top_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });

  VCDFileParser vcd_obj;
  spdlog::info("Loading Original Simulation Data");
  if (!boost::filesystem::exists(vcd_reference_file_path)) {
    throw std::runtime_error("File does not exist!!");
  }
  auto vcd_data = vcd_obj.parse_file(vcd_reference_file_path.string());

  auto stepsize = static_cast<uint32_t>(ducode::get_stepsize(vcd_data, designs.second));

  // get ift file size and number of simulation runs
  auto timesteps_per_simulation_run = tag_size / top_module->get_input_names().size();
  timesteps_per_simulation_run = std::max<size_t>(timesteps_per_simulation_run, 1);
  auto number_simulation_runs = ducode::get_number_of_simulation_runs(stepsize, timesteps_per_simulation_run, vcd_data);
  params["timesteps_per_simulation_run"] = timesteps_per_simulation_run;

  std::vector<boost::filesystem::path> exported_tags_files;
  for (uint32_t i = 0; i < number_simulation_runs; ++i) {
    boost::filesystem::path tag_file = temp_dir / ("vcd_input_" + std::to_string(i) + ".txt");
    exported_tags_files.push_back(tag_file);
  }
  //export the design with IFT
  design.write_verilog(exported_verilog, params);

  const VCDScope* root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));
  testbench.set_tag_generator(std::make_unique<ducode::FullResolutionTagGenerator>(top_module->get_input_ports(), vcd_data));

  std::vector<VCDTime> tags_times;
  for (auto time = 0u; time <= gsl::narrow<std::size_t>(vcd_data->get_timestamps().back()); time += stepsize) {
    tags_times.emplace_back(time);
  }

  testbench.write_verilog(exported_testbench, params);
  std::vector<ExportedTagsFile> exported_tags;

  /// here add loop which runs the simulation again for all created tag_map files.
  for (uint32_t i = 0; i < number_simulation_runs; ++i) {
    // For the ith batch of timestamps
    uint32_t items_per_batch = timesteps_per_simulation_run;
    uint32_t start_idx = i * items_per_batch;
    uint32_t end_idx = std::min(((i + 1) * items_per_batch), gsl::narrow<uint32_t>(tags_times.size()));

    std::vector<VCDTime> batch(tags_times.begin() + start_idx,
                               tags_times.begin() + end_idx);

    exported_tags.emplace_back(batch, exported_tags_files[i]);
  }
  testbench.write_tags(exported_tags, params);

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;
  for (const auto& exported_tags: exported_tags_files) {
    /// IFT simulation
    spdlog::info("Simulating Exported Design + Testbench");
    auto data = ducode::simulate_design(exported_verilog, exported_testbench, exported_tags);
    spdlog::info("End of Simulation");
    vcd_data_vector.emplace_back(data);
  }
  std::vector<const VCDScope*> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto& vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  ///fill json object with relevant information about design -> then add it to result data json
  nlohmann::json ift_res = ducode::do_analysis(instance, design);

  ift_res["number_of_injected_tags"] = params["number_of_injected_tags"];

  CHECK(ift_res["number_of_injected_tags"] == 9);
  CHECK(ift_res["output_tag_count"]["analysis_test_c"] == 3);

  CHECK(ift_res["output_tag_count"]["analysis_test_d"] == 6);

  CHECK(ift_res["output_tag_count"]["total_tag_count"] == 9);
}
