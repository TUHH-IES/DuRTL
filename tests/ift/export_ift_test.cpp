/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/ids.hpp>
#include <ducode/instantiation_graph.hpp>
#include <ducode/instantiation_graph_traits.hpp>
#include <ducode/module.hpp>
#include <ducode/signals_data_manager.hpp>
#include <ducode/tag_generator.hpp>
#include <ducode/testbench.hpp>
#include <ducode/utility/VCD_comparisons.hpp>
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
#include <vcd-parser/VCDFileParser.hpp>
#include <vcd-parser/VCDTypes.hpp>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////////////////////
// Tests if the design was written correctly as an extended IFT version and
// also if the exported versions show the same behavior to the original verilog code
///////////////////////////////////////////////////////////////////////////////////////////////

namespace {
std::tuple<ducode::Design, std::shared_ptr<VCDFile>, std::shared_ptr<VCDFile>> simulate_with_IFT(
    const std::string &file_name) {
  nlohmann::json params;
  params["ift"] = true;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;

  auto json_file = boost::filesystem::path{TESTFILES_DIR} / file_name / (file_name + ".json");
  auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / file_name / (file_name + ".vcd");
  auto reference_tb = boost::filesystem::path{TESTFILES_DIR} / file_name / (file_name + "_tb.v");
  auto reference_input_tags = boost::filesystem::path{TESTFILES_DIR} / file_name / (file_name + ".txt");
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  REQUIRE(created);
  auto verilog_export_file = temp_dir / "ift_design.v";

  auto design = ducode::Design::parse_json(json_file);

  spdlog::debug("temp_dir: {}", temp_dir.string());

  design.write_verilog(verilog_export_file, params);
  REQUIRE(boost::filesystem::exists(verilog_export_file));

  auto data = ducode::simulate_design(verilog_export_file, reference_tb, reference_input_tags);

  spdlog::debug("Load reference VCD FILE: {}", vcd_reference_file.string());

  VCDFileParser parser;
  auto vcd_reference_obj = parser.parse_file(vcd_reference_file.string());

  return {design, data, vcd_reference_obj};
}
}// namespace
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tests if the design was written back correctly as a verilog design that shows the same behavior to the original verilog code
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TEST_CASE("binary_gates_ift_test", "[ift_test]") {
  auto [design, simulation, reference] = simulate_with_IFT("binary_gates_ift_test");
  CHECK(*simulation == *reference);
}

TEST_CASE("unary_gates_ift_test", "[ift_test]") {
  auto [design, simulation, reference] = simulate_with_IFT("unary_gates_ift_test");
  CHECK(*simulation == *reference);
}

TEST_CASE("hierarchy_ift", "[ift_test]") {
  auto [design, simulation, reference] = simulate_with_IFT("hierarchy_ift");

  REQUIRE_FALSE(simulation->root_scope->children.empty());
  REQUIRE_FALSE(reference->root_scope->children.empty());

  auto *simulation_tb_scope = simulation->root_scope->children[0];
  auto *reference_tb_scope = reference->root_scope->children[0];

  CHECK(*simulation_tb_scope == *reference_tb_scope);

  bool contains_x = false;

  auto instance = ducode::DesignInstance::create_instance(design);
  const VCDScope *root_scope = &ducode::find_root_scope(simulation, design);
  ducode::VCDSignalsDataManager test_value_map(simulation, root_scope);
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));
  //instance.write_graphviz("hierarchy.dot");
  for (auto [edge_it, edge_end] = boost::edges(instance.m_graph); edge_it != edge_end; ++edge_it) {
    if (instance.m_graph[*edge_it].net_ptr != nullptr) {
      auto tag_values = instance.get_tag_values(*edge_it);
      for (const auto &tag_vector: tag_values) {
        for (const auto &tag_value: tag_vector.m_timed_signal_values) {
          for (const auto &tag_bit: tag_value.value.get_value_vector()) {
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

TEST_CASE("y86_ift", "[ift_test]") {
  nlohmann::json params;
  params["ift"] = true;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;

  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "y86" / "y86.json";
  auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / "y86" / "y86_gate_level.vcd";
  auto reference_tb = boost::filesystem::path{TESTFILES_DIR} / "y86" / "y86_tb.v";
  auto reference_ift_tb = boost::filesystem::path{TESTFILES_DIR} / "y86" / "y86_ift_tb.v";
  auto reference_input = boost::filesystem::path{TESTFILES_DIR} / "y86" / "y86.txt";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  REQUIRE(created);
  auto exported_design = temp_dir / "y86TestIFT.v";

  auto design = ducode::Design::parse_json(json_file);
  design.write_verilog(exported_design, params);

  REQUIRE(boost::filesystem::exists(exported_design));

  auto data = ducode::simulate_design(exported_design, reference_ift_tb, reference_input);

  REQUIRE(boost::filesystem::exists(vcd_reference_file));

  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_reference_file.string());

  //for (const ducode::Module &module: design.get_modules()) {
  //module.write_graphviz(module.get_name() + ".dot");
  //}


  // input output simulation should be enough for such a complex design.
  const auto &outer_scope = data->get_scope("y86_tb");
  const auto &outer_scope_reference = vcd_reference->get_scope("y86_tb");
  for (auto *scope_reference_signal: outer_scope_reference.signals) {
    auto signal_it = std::ranges::find_if(outer_scope.signals, [&](auto signal) {
      return signal->reference == scope_reference_signal->reference;
    });
    spdlog::info(scope_reference_signal->reference);
    CHECK(signal_it != outer_scope.signals.end());
    CHECK(vcd_reference->get_signal_values(scope_reference_signal->hash) == data->get_signal_values((*signal_it)->hash));
  }

  bool contains_x = false;
  const auto &signals = data->get_signals();
  for (const auto &module: design.get_modules()) {
    for (const auto &port: module.get_ports()) {
      auto signal_it = std::ranges::find_if(signals, [&](const VCDSignal &signal) {
        return signal.reference == (port.m_name + ift_tag);
      });
      REQUIRE(signal_it != signals.end());
      auto signal_values = data->get_signal_values(signal_it->hash);
      for (const auto &signal_value: signal_values) {
        for (const auto &tag_bit: signal_value.value.get_value_vector()) {
          if (tag_bit == VCDBit::VCD_X) {
            contains_x = true;
            spdlog::debug("{} contains X", signal_it->reference);
            break;
          }
        }
      }
    }
  }
  CHECK_FALSE(contains_x);
}

TEST_CASE("export eth_fifo with ift", "[ift_test]") {
  nlohmann::json params;
  params["ift"] = true;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;

  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "eth_fifo" / "eth_fifo.json";
  auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / "eth_fifo" / "eth_fifo.vcd";
  auto reference_ift_tb = boost::filesystem::path{TESTFILES_DIR} / "eth_fifo" / "eth_fifo_ift_tb.v";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  REQUIRE(created);
  REQUIRE(boost::filesystem::exists(vcd_reference_file));

  auto design = ducode::Design::parse_json(json_file);
  auto exported_design = temp_dir / "exported_design.v";
  design.write_verilog(exported_design, params);
  REQUIRE(boost::filesystem::exists(exported_design));

  auto data = ducode::simulate_design(exported_design, reference_ift_tb);

  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_reference_file.string());

  const auto &outer_scope = data->get_scope("eth_fifo_tb");
  const auto &outer_scope_reference = vcd_reference->get_scope("eth_fifo_tb");
  for (auto *scope_reference_signal: outer_scope_reference.signals) {
    auto signal_it = std::ranges::find_if(outer_scope.signals, [&](auto signal) {
      return signal->reference == scope_reference_signal->reference;
    });
    spdlog::info(scope_reference_signal->reference);
    REQUIRE(signal_it != outer_scope.signals.end());
    if (scope_reference_signal->type != VCDVarType::VCD_VAR_PARAMETER) {
      CHECK(vcd_reference->get_signal_values(scope_reference_signal->hash) == data->get_signal_values((*signal_it)->hash));
    }
  }

  bool contains_x = false;
  const auto &signals = data->get_signals();
  for (const auto &module: design.get_modules()) {
    for (const auto &port: module.get_ports()) {
      auto signal_it = std::ranges::find_if(signals, [&](const VCDSignal &signal) {
        return signal.reference == (port.m_name + ift_tag);
      });
      REQUIRE(signal_it != signals.end());
      auto signal_values = data->get_signal_values(signal_it->hash);
      for (const auto &signal_value: signal_values) {
        for (const auto &tag_bit: signal_value.value.get_value_vector()) {
          if (tag_bit == VCDBit::VCD_X) {
            contains_x = true;
            spdlog::debug("{} contains X", signal_it->reference);
            break;
          }
        }
      }
    }
  }
  CHECK_FALSE(contains_x);
}

TEST_CASE("counter_ift", "[ift_test]") {
  nlohmann::json params;
  params["ift"] = true;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;


  auto json_file = std::filesystem::path(TESTFILES_DIR) / "counter" / "counter.json";
  auto design = ducode::Design::parse_json(json_file);

  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  REQUIRE(created);

  auto instrumented_verilog = temp_dir / "iftverilog.v";
  design.write_verilog(instrumented_verilog, params);

  CHECK(boost::filesystem::exists(instrumented_verilog));

  auto design_file = std::filesystem::path(TESTFILES_DIR) / "counter" / "counter.v";
  auto testbench_file = std::filesystem::path(TESTFILES_DIR) / "counter" / "counter_tb.v";
  auto vcd_data = ducode::simulate_design(design_file, testbench_file);

  auto vcd_reference_file = std::filesystem::path(TESTFILES_DIR) / "counter" / "counter.vcd";
  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_reference_file.string());
  CHECK(ducode::no_repetitions_compare(*vcd_reference, *vcd_data));

  auto exported_testbench = temp_dir / "iftVerilog_tb.v";

  auto timesteps_per_simulation_run = tag_size / design.get_module(design.get_top_module_name()).get_input_names().size();
  timesteps_per_simulation_run = std::max<size_t>(timesteps_per_simulation_run, 1);
  params["timesteps_per_simulation_run"] = timesteps_per_simulation_run;

  const VCDScope *root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));
  testbench.write_verilog(exported_testbench, params);

  CHECK(boost::filesystem::exists(exported_testbench));

  auto vcd_data_ift = ducode::simulate_design(instrumented_verilog, exported_testbench);

  bool contains_x = false;
  const auto &signals = vcd_data_ift->get_signals();
  for (const auto &module: design.get_modules()) {
    for (const auto &port: module.get_ports()) {
      auto signal_it = std::ranges::find_if(signals, [&](const VCDSignal &signal) {
        return signal.reference == (port.m_name + ift_tag);
      });
      REQUIRE(signal_it != signals.end());
      auto signal_values = vcd_data_ift->get_signal_values(signal_it->hash);
      for (const auto &signal_value: signal_values) {
        for (const auto &tag_bit: signal_value.value.get_value_vector()) {
          if (tag_bit == VCDBit::VCD_X) {
            contains_x = true;
            spdlog::debug("{} contains X", signal_it->reference);
            break;
          }
        }
      }
    }
  }
  CHECK_FALSE(contains_x);
}

// checking the tag vectors for 'x' values
TEST_CASE("complete_ift_y86", "[ift_tag_x_test][.]") {
  nlohmann::json params;
  params["ift"] = true;
  const uint32_t tag_size = 2048;
  params["tag_size"] = tag_size;
  const std::string design_name = "y86";
  const std::string signal_name = "clk";

  auto verilog_file = boost::filesystem::path(TESTFILES_DIR) / design_name / (design_name + ".v");
  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / design_name / (design_name + "_tb.v");
  auto json_file = boost::filesystem::path(TESTFILES_DIR) / design_name / (design_name + ".json");
  auto reference_input = boost::filesystem::path{TESTFILES_DIR} / "y86" / "test_0.txt";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  auto design = ducode::Design::parse_json(json_file);

  auto vcd_data = ducode::simulate_design(verilog_file, testbench_file, reference_input);

  auto exported_verilog = temp_dir / "ift_design.v";
  auto exported_testbench = temp_dir / "ift_testbench.v";

  design.write_verilog(exported_verilog, params);

  auto top_module = std::ranges::find_if(design.get_modules(), [&](const auto &mod) {
    return mod.get_name() == design.get_top_module_name();
  });
  REQUIRE(top_module != design.get_modules().end());

  auto stepsize = gsl::narrow<uint32_t>(ducode::get_stepsize(vcd_data, signal_name, "dut"));

  auto timesteps_per_simulation_run = get_timesteps_per_simulation_run(*top_module, tag_size);
  params["timesteps_per_simulation_run"] = timesteps_per_simulation_run;

  auto number_simulation_runs = ducode::get_number_of_simulation_runs(stepsize, timesteps_per_simulation_run, vcd_data);

  spdlog::info("Running {} simulations", number_simulation_runs);

  std::vector<boost::filesystem::path> exported_tags_files = ducode::get_exported_tags_files(number_simulation_runs);

  const VCDScope *root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));
  testbench.set_tag_generator(
      std::make_unique<ducode::FullResolutionTagGenerator>(top_module->get_input_ports(), vcd_data));

  std::vector<VCDTime> tags_times = ducode::get_tags_times(vcd_data, stepsize);

  testbench.write_verilog(exported_testbench, params);
  auto exported_tags_vec =
      ducode::get_exported_tags_vec(
          exported_tags_files, number_simulation_runs, tags_times, timesteps_per_simulation_run);

  testbench.write_tags(exported_tags_vec, params);

  spdlog::info("Simulating Exported Design + Testbench");

  auto instance = ducode::DesignInstance::create_instance(design);

  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;
  for (const auto &exported_tags: exported_tags_files) {
    /// IFT simulation
    spdlog::info("Simulating Exported Design + Testbench");
    auto data = ducode::simulate_design(exported_verilog, exported_testbench, exported_tags);
    spdlog::info("End of Simulation");
    vcd_data_vector.emplace_back(data);
  }
  std::vector<const VCDScope *> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto &vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  bool contains_x = false;

  for (auto [edge_it, edge_end] = boost::edges(instance.m_graph); edge_it != edge_end; ++edge_it) {
    if (instance.m_graph[*edge_it].net_ptr != nullptr) {
      auto tag_values = instance.get_tag_values(*edge_it);
      for (const auto &tag_vector: tag_values) {
        for (const auto &tag_value: tag_vector.m_timed_signal_values) {
          for (const auto &tag_bit: tag_value.value.get_value_vector()) {
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

TEST_CASE("complete_ift_aes128", "[ift_tag_x_test][.]") {
  nlohmann::json params;
  params["ift"] = true;
  const int tag_size = 128;// 128 bits
  params["tag_size"] = tag_size;
  const std::vector<std::pair<std::string, std::string>> designs = {{"aes_128", "clk"}};

  const std::string design_name = "aes_128";

  auto verilog_file = boost::filesystem::path(TESTFILES_DIR) / design_name / (design_name + ".v");
  auto testbench_file = boost::filesystem::path{TESTFILES_DIR} / design_name / (design_name + "_tb.v");
  auto json_file = boost::filesystem::path(TESTFILES_DIR) / design_name / (design_name + ".json");
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);

  auto design = ducode::Design::parse_json(json_file);

  auto exported_verilog = temp_dir / "ift_design.v";
  auto exported_testbench = temp_dir / "ift_testbench.v";

  design.write_verilog(exported_verilog, params);

  auto top_module = std::ranges::find_if(design.get_modules(), [&](const auto &mod) {
    return mod.get_name() == design.get_top_module_name();
  });
  REQUIRE(top_module != design.get_modules().end());

  auto vcd_data = ducode::simulate_design(verilog_file, testbench_file);

  auto stepsize = gsl::narrow<uint32_t>(ducode::get_stepsize(vcd_data, designs[0].second, "uut"));

  auto timesteps_per_simulation_run = get_timesteps_per_simulation_run(*top_module, tag_size);
  params["timesteps_per_simulation_run"] = timesteps_per_simulation_run;

  auto number_simulation_runs = ducode::get_number_of_simulation_runs(stepsize, timesteps_per_simulation_run, vcd_data);

  spdlog::info("Running {} simulations", number_simulation_runs);

  std::vector<boost::filesystem::path> exported_tags_files = ducode::get_exported_tags_files(number_simulation_runs);

  const VCDScope *root_scope = &ducode::find_root_scope(vcd_data, design);
  ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
  ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));
  testbench.set_tag_generator(
      std::make_unique<ducode::FullResolutionTagGenerator>(top_module->get_input_ports(), vcd_data));

  std::vector<VCDTime> tags_times = ducode::get_tags_times(vcd_data, stepsize);

  testbench.write_verilog(exported_testbench, params);
  auto exported_tags_vec =
      ducode::get_exported_tags_vec(
          exported_tags_files, number_simulation_runs, tags_times, timesteps_per_simulation_run);

  testbench.write_tags(exported_tags_vec, params);

  spdlog::info("Simulating Exported Design + Testbench");

  auto instance = ducode::DesignInstance::create_instance(design);
  std::vector<std::shared_ptr<VCDFile>> vcd_data_vector;
  for (const auto &exported_tags: exported_tags_files) {
    /// IFT simulation
    spdlog::info("Simulating Exported Design + Testbench");
    auto data = ducode::simulate_design(exported_verilog, exported_testbench, exported_tags);
    spdlog::info("End of Simulation");
    vcd_data_vector.emplace_back(data);
  }
  std::vector<const VCDScope *> vcd_root_scopes;
  vcd_root_scopes.reserve(vcd_data_vector.size());
  for (auto &vcd_file: vcd_data_vector) {
    vcd_root_scopes.emplace_back(&ducode::find_root_scope(vcd_file, design));
  }
  ducode::VCDSignalsDataManager test_value_map(vcd_data_vector, vcd_root_scopes, vcd_data_vector.size());
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));

  bool contains_x = false;

  for (auto [edge_it, edge_end] = boost::edges(instance.m_graph); edge_it != edge_end; ++edge_it) {
    if (instance.m_graph[*edge_it].net_ptr != nullptr) {
      auto tag_values = instance.get_tag_values(*edge_it);
      for (const auto &tag_vector: tag_values) {
        for (const auto &tag_value: tag_vector.m_timed_signal_values) {
          for (const auto &tag_bit: tag_value.value.get_value_vector()) {
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
