/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/generate_tags.hpp>
#include <ducode/instantiation_graph.hpp>
#include <ducode/testbench.hpp>
#include <ducode/utility/iverilog_wrapper.hpp>

#include <boost/filesystem.hpp>
#include <catch2/catch_all.hpp>
#include <spdlog/spdlog.h>
#include <vcd-parser/VCDComparisons.hpp>

#include <filesystem>
#include <string>

// comparing the original design with design exported by yosys
TEST_CASE("benchmark_yosys_compare", "[benchmarks][.]") {}

// comparing our exported design with design exported by yosys
TEST_CASE("benchmark_export", "[benchmarks][.]") {}

// testing our IFT flow for all benchmark designs with check if any tags contain X's
TEST_CASE("benchmark_xs", "[benchmarks][.]") {
  nlohmann::json params;

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
    auto exported_tags = temp_dir / "vcd_input.txt";

    auto design = ducode::Design::parse_json(json_file);

    design.write_verilog(exported_verilog, params);

    // original design Simulation
    spdlog::info("Simulating Original Design + Testbench");
    auto vcd_data = ducode::simulate_design(exported_verilog, testbench_file);
    spdlog::info("End of Simulation");

    params["ift"] = true;

    design.write_verilog(exported_ift_verilog, params);
    CHECK(!boost::filesystem::is_empty(exported_ift_verilog));


    //get top module
    auto tb_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });

    auto tag_map = generate_tags_random(vcd_data->get_timestamps().back(), des.second, random_tag_number, tb_module->get_ports());
    uint32_t tag_cnt = 0;
    for (auto& tags: tag_map) {
      for (auto& ts: tags.second) {
        tag_cnt++;
      }
    }
    CHECK(tag_cnt == random_tag_number);
    spdlog::info("ANZAHL TAGS IN DER TAG MAP NACH RANDOM GENERATION ::: " + std::to_string(tag_cnt) + " !!!");

    std::vector<std::map<uint32_t, std::pair<uint32_t, ducode::Port>>> encoded_tags = tag_processing(tb_module, tag_map);
    spdlog::info("Running {} simulations", encoded_tags.size());
    export_tags(tb_module, exported_tags.string(), encoded_tags[0]);
    CHECK(!boost::filesystem::is_empty(exported_tags));

    //exporting testbench with IFT
    const ducode::Testbench testbench(design, vcd_data);
    testbench.write_verilog(exported_testbench, exported_tags, params);
    /// check testbench not empty!
    CHECK(!boost::filesystem::is_empty(exported_testbench));

    /// IFT simulation
    spdlog::info("Simulating Exported Design + Testbench");
    auto data = ducode::simulate_design(exported_ift_verilog, exported_testbench, exported_tags);
    spdlog::info("End of Simulation");

    /// create instantiation graph with simulation results
    auto instance = ducode::DesignInstance::create_instance(design);
    instance.add_vcd_data(data);

    //      export_tags(tb_module, exported_tags.string(), encoded_tags[1]);
    //      testbench.write_verilog(exported_testbench, exported_tags, params);

    /// here add loop which runs the simulation again for all created tag_map files.
    for (auto& tags: encoded_tags) {
      if (tags == encoded_tags[0]) {
        continue;
      }
      export_tags(tb_module, exported_tags.string(), tags);
      testbench.write_verilog(exported_testbench, exported_tags, params);

      /// IFT simulation
      spdlog::info("Simulating Exported Design + Testbench");
      data = ducode::simulate_design(exported_ift_verilog, exported_testbench, exported_tags);
      spdlog::info("End of Simulation");
      instance.add_vcd_data(data);
    }

    bool contains_x = false;

    for (auto [edge_it, edge_end] = boost::edges(instance.m_graph); edge_it != edge_end; ++edge_it) {
      if (instance.m_graph[*edge_it].net_ptr != nullptr) {
        auto tag_values = instance.get_tag_values(*edge_it);
        for (const auto* tag_vector: tag_values) {
          for (const auto& tag_value: *tag_vector) {
            for (const auto& tag_bit: tag_value.value.get_value_vector()) {
              if (tag_bit == VCDBit::VCD_X) {
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