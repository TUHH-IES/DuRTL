/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/generate_tags.hpp>
#include <ducode/instantiation_graph.hpp>
#include <ducode/sim.hpp>
#include <ducode/testbench.hpp>
#include <ducode/utility/iverilog_wrapper.hpp>

#include <boost/filesystem.hpp>
#include <catch2/catch_all.hpp>
#include <spdlog/spdlog.h>
#include <vcd-parser/VCDComparisons.hpp>

#include <string>

/// TESTS ///
/*
 * ift_test
 * check output signals of larger designs for any x-values and tags
 */

void run_ift(const std::string& design_name, uint32_t stepsize = 1) {
  nlohmann::json params;

  params["ift"] = false;
  params["apprx"] = false;

  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / design_name / (design_name + "_tb.v");
  auto json_file = std::filesystem::path(TESTFILES_DIR) / design_name / (design_name + ".json");

  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  auto exported_verilog = temp_dir / ("ift" + design_name + ".v");
  auto exported_testbench = temp_dir / ("ift" + design_name + "_tb.v");
  auto exported_tags = temp_dir / "vcd_input.txt";

  auto design = ducode::Design::parse_json(json_file);

  //export the design without IFT
  design.write_verilog(exported_verilog, params);

  // original design Simulation
  auto vcd_data = ducode::simulate_design(exported_verilog, testbench_file);

  params["ift"] = true;

  //export the design with IFT
  design.write_verilog(exported_verilog, params);

  //get top module
  auto tb_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });

  auto instance = ducode::DesignInstance::create_instance(design);

  //create tags with full resolution
  auto tag_map = generate_tags_full_resolution(vcd_data->get_timestamps().back(), stepsize, tb_module->get_ports());

  // only using s single simulation run and corresponding tagging file (tag_map[0]) for testbench generation
  std::vector<std::map<uint32_t, std::pair<uint32_t, ducode::Port>>> encoded_tags = tag_processing(tb_module, tag_map);
  spdlog::info("ES WERDEN {} SIMULATIONEN AUSGEFÜHRT", encoded_tags.size());
  export_tags(tb_module, exported_tags.string(), encoded_tags[0]);

  bool contains_tags = false;
  bool contains_x = false;
  //exporting testbench with IFT
  const ducode::Testbench testbench(design, vcd_data);
  testbench.write_verilog(exported_testbench, exported_tags, params);

  /// IFT simulation
  auto data = ducode::simulate_design(exported_verilog, exported_testbench, exported_tags);

  instance.add_vcd_data(data);

  for (auto& vertex: instance.m_graph.m_vertices) {
    if (vertex.m_property.type == "module_port") {
      for (auto& edge: vertex.m_in_edges) {
        for (const auto* tag_vector: instance.get_tag_values(edge)) {
          for (const auto& tag_value: *tag_vector) {
            for (const auto& tgs: tag_value.value.get_value_vector()) {
              if (tgs == VCDBit::VCD_1) {
                contains_tags = true;
                break;
              }
            }
            for (const auto& tgs: tag_value.value.get_value_vector()) {
              if (tgs == VCDBit::VCD_X) {
                contains_x = true;
                break;
              }
            }
          }
        }
      }
    }
  }

  if (encoded_tags.size() > 1) {
    //exporting testbench with IFT again because from the second simulation on the testbench due to tag files having one more line
    export_tags(tb_module, exported_tags.string(), encoded_tags[1]);
  }
  testbench.write_verilog(exported_testbench, exported_tags, params);

  /// here add loop which runs the simulation again for all created tag_map files.
  for (auto& tags: encoded_tags) {
    if (tags == encoded_tags[0]) {
      continue;
    }
    export_tags(tb_module, exported_tags.string(), tags);

    /// IFT simulation
    spdlog::info("Simulating Exported Design + Testbench");
    data = ducode::simulate_design(exported_verilog, exported_testbench, exported_tags);
    instance.add_vcd_data(data);

    for (auto& vertex: instance.m_graph.m_vertices) {
      if (vertex.m_property.type == "module_port") {
        for (auto& edge: vertex.m_in_edges) {
          for (const auto* tag_vector: instance.get_tag_values(edge)) {
            for (const auto& tag_value: *tag_vector) {
              for (const auto& tgs: tag_value.value.get_value_vector()) {
                if (tgs == VCDBit::VCD_1) {
                  contains_tags = true;
                  break;
                }
              }
              for (const auto& tgs: tag_value.value.get_value_vector()) {
                if (tgs == VCDBit::VCD_X) {
                  contains_x = true;
                  break;
                }
              }
            }
          }
        }
      }
    }
  }

  CHECK(contains_tags);
  CHECK(!contains_x);
}

// testet ob mindestens eine 1 an einem output eines hardware designs ankommen und das kein x-value an einem output ankommt
TEST_CASE("ift_test", "[ift_test]") {
  const uint32_t SPI_Master_stepsize = 20;
  run_ift("aes_module_t", 1);

  //  run_ift("SPI_Master", SPI_Master_stepsize);
  //  run_ift("SPI_Slave", 200);
  //  run_ift("picorv32", 5000);
}


TEST_CASE("ift_test_fft_apprx", "[ift_test][.]") {
  nlohmann::json params;
  params["ift"] = true;
  params["apprx"] = true;

  const int stepsize = 500;
  auto json_file = std::filesystem::path(TESTFILES_DIR) / "fft_appx_add" / "fft16_apx.json";
  auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / "fft_appx_add" / "tb_fft16_ofdm_apx.vcd";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  auto design = ducode::Design::parse_json(json_file);
  VCDFileParser vcd_obj;
  auto vcd_data = vcd_obj.parse_file(vcd_reference_file_path.string());

  auto exported_verilog = temp_dir / ("ift_design.v");
  auto exported_testbench = temp_dir / "ift_testbench.v";
  auto exported_tags = temp_dir / "vcd_input.txt";

  auto tb_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });
  //  auto tag_map = generate_tags_full_resolution(tb_module, vcd_data->get_timestamps().back(), stepsize);
  auto tag_map = generate_tags_random(vcd_data->get_timestamps().back(), stepsize, 0, tb_module->get_ports());

  design.write_verilog(exported_verilog, params);

  auto instance = ducode::do_simulation(design, tb_module, vcd_data, exported_verilog, exported_testbench, exported_tags, tag_map, params);

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

TEST_CASE("ift_test_fft_eta2", "[ift_test][.]") {
  nlohmann::json params;
  params["ift"] = true;
  params["apprx"] = true;

  const int stepsize = 500;
  auto json_file = std::filesystem::path(TESTFILES_DIR) / "fft_eta2" / "fft16_eta2.json";
  auto vcd_reference_file_path = boost::filesystem::path{TESTFILES_DIR} / "fft_eta2" / "fft16_eta2.vcd";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  ducode::Design design = ducode::Design::parse_json(json_file);
  VCDFileParser vcd_obj;
  auto vcd_data = vcd_obj.parse_file(vcd_reference_file_path.string());

  auto exported_verilog = temp_dir / "ift_design.v";
  auto exported_testbench = temp_dir / "ift_testbench.v";
  auto exported_tags = temp_dir / "vcd_input.txt";

  auto tb_module = std::ranges::find_if(design.get_modules(), [&](const auto& mod) { return mod.get_name() == design.get_top_module_name(); });
  //  auto tag_map = generate_tags_full_resolution(tb_module, vcd_data->get_timestamps().back(), stepsize);
  const uint64_t max_tag_count = 100;
  auto tag_map = generate_tags_random(vcd_data->get_timestamps().back(), stepsize, max_tag_count, tb_module->get_ports());

  design.write_verilog(exported_verilog, params);

  //need to add map of tagged_ports + conditions
  //  DesignInstance instance = ducode::do_simulation(design, tb_module, vcd_data, exported_verilog, exported_testbench, exported_tags, tag_map, params);
  std::unordered_map<std::string, std::string> tagged_ports = {{"tag_add1_1", "err_add1_1"}, {"tag_add1_2", "err_add1_2"}, {"tag_add2_1", "err_add2_1"}, {"tag_add2_2", "err_add2_2"}, {"tag_sub1_1", "err_sub1_1"}, {"tag_sub1_2", "err_sub1_2"}, {"tag_sub2_1", "err_sub2_1"}, {"tag_sub2_2", "err_sub2_2"}};
  ducode::DesignInstance instance = ducode::do_simulation(design, vcd_data, exported_verilog, exported_testbench, tagged_ports, params);


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