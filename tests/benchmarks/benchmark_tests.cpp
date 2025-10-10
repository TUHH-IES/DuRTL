/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/instantiation_graph.hpp>
#include <ducode/instantiation_graph_traits.hpp>
#include <ducode/signals_data_manager.hpp>
#include <ducode/tag_generator.hpp>
#include <ducode/testbench.hpp>
#include <ducode/utility/iverilog_wrapper.hpp>
#include <ducode/utility/simulation.hpp>

#include "ducode/utility/VCD_utility.hpp"
#include <boost/filesystem/operations.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/detail/adjacency_list.hpp>
#include <catch2/catch_test_macros.hpp>
#include <gsl/narrow>
#include <nlohmann/json_fwd.hpp>
#include <spdlog/spdlog.h>
#include <vcd-parser/VCDFile.hpp>
#include <vcd-parser/VCDTypes.hpp>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// comparing the original design with design exported by yosys
TEST_CASE("benchmark_yosys_compare", "[benchmarks][.]") {
}

// comparing our exported design with design exported by yosys
TEST_CASE("benchmark_export", "[benchmarks][.]") {
}

// testing our IFT flow for all benchmark designs with check if any tags contain X's
TEST_CASE("benchmark_xs", "[benchmarks][.]") {
  nlohmann::json params;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;

  // each pair contains the name of a design and the corresponding simulation stepsize for tag injection
  // const std::vector<std::pair<std::string, unsigned int>> designs = {{"y86", 10}, {"SPI_Master", 20}, {"SPI_Slave", 200}, {"picorv32", 5000}, {"aes_128", 5000}};
  const std::vector<std::pair<std::string, unsigned int>> designs = {{"SPI_Slave", 200}};
  std::map<uint32_t, std::vector<std::pair<std::string, uint32_t>>> tag_count_map;
  nlohmann::json result_data_random;
  const int random_tag_number = 100;
  std::map<std::string, std::vector<uint32_t>> tag_counts;
  for (const auto& des: designs) {
    params["ift"] = false;

    auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / des.first / (des.first + "_tb.v");
    auto json_file = std::filesystem::path(TESTFILES_DIR) / des.first / (des.first + ".json");

    auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
    auto created = boost::filesystem::create_directories(temp_dir);
    assert(created);

    auto exported_ift_verilog = temp_dir / ("ift" + des.first + ".v");
    auto exported_verilog = temp_dir / (des.first + ".v");
    auto exported_testbench = temp_dir / ("ift" + des.first + "_tb.v");

    auto design = ducode::Design::parse_json(json_file);

    design.write_verilog(exported_verilog, params);

    // original design Simulation
    spdlog::debug("Simulating Original Design + Testbench");
    auto vcd_data = ducode::simulate_design(exported_verilog, testbench_file);
    spdlog::debug("End of Simulation");

    params["ift"] = true;

    design.write_verilog(exported_ift_verilog, params);
    CHECK(!boost::filesystem::is_empty(exported_ift_verilog));

    //get top module
    auto top_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) {
      return mod.get_name() == design.get_top_module_name();
    });

    const auto stepsize = params.value("stepsize", 1);
    const auto timesteps_per_simulation_run = ducode::get_timesteps_per_simulation_run(*top_module, tag_size);
    const auto number_simulation_runs = ducode::get_number_of_simulation_runs(stepsize, timesteps_per_simulation_run,
                                                                              vcd_data);

    //exporting testbench with IFT
    const VCDScope* root_scope = &ducode::find_root_scope(vcd_data, design);
    ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
    ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));
    testbench.write_verilog(exported_testbench, params);
    /// check testbench not empty!
    const uint32_t coverage = 50;
    testbench.set_tag_generator(
        std::make_unique<ducode::DeterministicRandomTagGenerator>(top_module->get_input_ports(), vcd_data, coverage));
    testbench.write_verilog(exported_testbench, params);

    auto exported_tags_files = ducode::get_exported_tags_files(number_simulation_runs);
    auto tags_times = ducode::get_tags_times(vcd_data, gsl::narrow<uint32_t>(stepsize));
    auto exported_tags_vec = ducode::get_exported_tags_vec(exported_tags_files, number_simulation_runs,
                                                           tags_times, timesteps_per_simulation_run);
    testbench.write_tags(exported_tags_vec, params);
    spdlog::info("Number of tags: {}", testbench.get_number_of_injected_tags());
    CHECK(!boost::filesystem::is_empty(exported_testbench));

    CHECK(testbench.get_number_of_injected_tags() == random_tag_number);

    auto instance = ducode::DesignInstance::create_instance(design);

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

    bool contains_x = false;

    for (auto [edge_it, edge_end] = boost::edges(instance.m_graph); edge_it != edge_end; ++edge_it) {
      if (instance.m_graph[*edge_it].net_ptr != nullptr) {
        auto tag_values = instance.get_tag_values(*edge_it);
        for (const auto& tag_vector: tag_values) {
          for (const auto& tag_value: tag_vector.m_timed_signal_values) {
            for (const auto& tag_bit: tag_value.value.get_value_vector()) {
              if (tag_bit == ducode::SignalBit::BIT_X) {
                contains_x = true;
                break;
              }
            }
          }
        }
      }
    }
    CHECK_FALSE(contains_x);
  }
}
