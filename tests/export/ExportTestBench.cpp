/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/instantiation_graph.hpp>
#include <ducode/signals_data_manager.hpp>
#include <ducode/testbench.hpp>
#include <ducode/utility/VCD_comparisons.hpp>
#include <ducode/utility/VCD_utility.hpp>
#include <ducode/utility/iverilog_wrapper.hpp>

#include <boost/filesystem/operations.hpp>
#include <catch2/catch_test_macros.hpp>
#include <fmt/core.h>
#include <nlohmann/json_fwd.hpp>
#include <spdlog/spdlog.h>
#include <vcd-parser/VCDComparisons.hpp>// NOLINT(misc-include-cleaner)
#include <vcd-parser/VCDFile.hpp>
#include <vcd-parser/VCDFileParser.hpp>
#include <vcd-parser/VCDTypes.hpp>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

TEST_CASE("testbench export for basic gates", "[testbench]") {
  nlohmann::json params;
  params["ift"] = false;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;

  std::vector<std::string> design_names = {"andGate", "orGate", "xorGate", "andVector", "mux", "logic_andGate", "logic_orGate", "LeCell", "GeCell", "EqCell", "LtCell", "GtCell", "reduceAndGate", "reduceOrGate", "reduceXorGate", "reduceXnorGate", "posGate"};

  for (const auto& design_name: design_names) {

    auto json_file = std::filesystem::path{TESTFILES_DIR} / "flattened_verilog" / (design_name + ".json");
    auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / "flattened_verilog" / (design_name + ".vcd");
    auto design_file = boost::filesystem::path{TESTFILES_DIR} / "flattened_verilog" / (design_name + ".v");
    auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
    auto created = boost::filesystem::create_directories(temp_dir);
    assert(created);
    auto tb_file = temp_dir / (design_name + "_tb.v");

    auto design = ducode::Design::parse_json(json_file);

    VCDFileParser parser;
    auto vcd_data = parser.parse_file(vcd_reference_file.string());

    const VCDScope* root_scope = &ducode::find_root_scope(vcd_data, design);
    ducode::VCDSignalsDataManager testbench_value_map(vcd_data, root_scope);
    const ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));

    testbench.write_verilog(tb_file, params);

    REQUIRE(boost::filesystem::exists(tb_file));

    auto data = ducode::simulate_design(design_file, tb_file);

    std::shared_ptr<VCDFile> vcd_reference = parser.parse_file(vcd_reference_file.string());

    //comparing vcd outputfiles checks if files are the same
    const auto& inner_scope = data->get_scope(fmt::format("{}_tb", design_name));
    const auto& inner_scope_reference = vcd_reference->get_scope(fmt::format("{}_tb", design_name));
    for (auto* scope_reference_signal: inner_scope_reference.signals) {
      auto signal_it = std::ranges::find_if(inner_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
      CHECK(signal_it != inner_scope.signals.end());
      CHECK(vcd_reference->get_signal_values(scope_reference_signal->hash) == data->get_signal_values((*signal_it)->hash));
    }
  }
}

// tests if the exported testbench for the aes_128 design is correct is giving the same output as the reference testbench
TEST_CASE("testbench export for aes_128", "[export]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "aes_128" / "aes_128.json";
  auto reference_tb = boost::filesystem::path{TESTFILES_DIR} / "aes_128" / "aes_128_tb.v";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  REQUIRE(created);
  auto exported_design = temp_dir / "ift_aes_128.v";
  auto exported_testbench = temp_dir / "ift_aes_128_tb.v";

  nlohmann::json params;
  params["ift"] = false;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;

  auto design = ducode::Design::parse_json(json_file);
  design.write_verilog(exported_design, params);
  REQUIRE(boost::filesystem::exists(exported_design));

  auto data_reference = ducode::simulate_design(exported_design, reference_tb);

  const VCDScope* root_scope = &ducode::find_root_scope(data_reference, design);
  ducode::VCDSignalsDataManager testbench_value_map(data_reference, root_scope);
  const ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));
  testbench.write_verilog(exported_testbench, params);

  auto data = ducode::simulate_design(exported_design, exported_testbench);

  ducode::DesignInstance instance = ducode::DesignInstance::create_instance(design);
  const VCDScope* root_scope_2 = &ducode::find_root_scope(data, design);
  ducode::VCDSignalsDataManager test_value_map(data, root_scope_2);

  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));
  //instance.write_graphviz("aes_128.dot");

  // input output simulation should be enough for such a complex design.
  const auto& outer_scope = data->get_scope("aes_128_tb");
  const auto& outer_scope_reference = data_reference->get_scope("aes_128_tb");
  for (auto* scope_reference_signal: outer_scope_reference.signals) {
    spdlog::info("{}", scope_reference_signal->reference);
    auto signal_it = std::ranges::find_if(outer_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    REQUIRE(signal_it != outer_scope.signals.end());
    CHECK(ducode::no_repetitions_compare(data_reference->get_signal_values(scope_reference_signal->hash), data->get_signal_values((*signal_it)->hash)));
  }

  const auto& dut_scope = data->get_scope("Ins");
  const auto& dut_scope_reference = data_reference->get_scope("uut");
  for (auto* scope_reference_signal: dut_scope_reference.signals) {
    auto signal_it = std::ranges::find_if(dut_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    if (signal_it != dut_scope.signals.end()) {
      spdlog::info("{}", scope_reference_signal->reference);
      CHECK(ducode::no_repetitions_compare(data_reference->get_signal_values(scope_reference_signal->hash), data->get_signal_values((*signal_it)->hash)));
    }
  }
}

TEST_CASE("Export y86_tb", "[export][.]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "y86" / "y86.json";
  auto reference_tb = boost::filesystem::path{TESTFILES_DIR} / "y86" / "y86_tb.v";
  auto reference_input = boost::filesystem::path{TESTFILES_DIR} / "y86" / "test_0.txt";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  REQUIRE(created);
  auto exported_design = temp_dir / "ift_y86.v";
  auto exported_testbench = temp_dir / ("ift_y86_tb.v");

  nlohmann::json params;
  params["ift"] = false;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;

  auto design = ducode::Design::parse_json(json_file);
  design.write_verilog(exported_design, params);
  REQUIRE(boost::filesystem::exists(exported_design));

  auto data_reference = ducode::simulate_design(exported_design, reference_tb, reference_input);

  const VCDScope* root_scope = &ducode::find_root_scope(data_reference, design);
  ducode::VCDSignalsDataManager testbench_value_map(data_reference, root_scope);
  const ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));
  testbench.write_verilog(exported_testbench, params);

  auto data = ducode::simulate_design(exported_design, exported_testbench);

  ducode::DesignInstance instance = ducode::DesignInstance::create_instance(design);
  const VCDScope* root_scope_2 = &ducode::find_root_scope(data, design);
  ducode::VCDSignalsDataManager test_value_map(data, root_scope_2);

  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));
  //instance.write_graphviz("y86.dot");

  // input output simulation should be enough for such a complex design.
  const auto& outer_scope = data->get_scope("cisc_cpu_p_tb");
  const auto& outer_scope_reference = data_reference->get_scope("y86_tb");
  for (auto* scope_reference_signal: outer_scope_reference.signals) {
    spdlog::info("{}", scope_reference_signal->reference);
    auto signal_it = std::ranges::find_if(outer_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    REQUIRE(signal_it != outer_scope.signals.end());
    CHECK(ducode::no_repetitions_compare(data_reference->get_signal_values(scope_reference_signal->hash), data->get_signal_values((*signal_it)->hash)));
  }

  const auto& dut_scope = data->get_scope("Ins");
  const auto& dut_scope_reference = data_reference->get_scope("dut");
  for (auto* scope_reference_signal: dut_scope_reference.signals) {
    auto signal_it = std::ranges::find_if(dut_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    if (signal_it != dut_scope.signals.end()) {
      spdlog::info("{}", scope_reference_signal->reference);
      CHECK(ducode::no_repetitions_compare(data_reference->get_signal_values(scope_reference_signal->hash), data->get_signal_values((*signal_it)->hash)));
    }
  }
}

TEST_CASE("Export SPI_Master_tb", "[export][.]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "SPI_Master" / "SPI_Master.json";
  auto reference_tb = boost::filesystem::path{TESTFILES_DIR} / "SPI_Master" / "SPI_Master_tb.v";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  REQUIRE(created);
  auto exported_design = temp_dir / "ift_SPI_Master.v";
  auto exported_testbench = temp_dir / "ift_SPI_Master_tb.v";

  nlohmann::json params;
  params["ift"] = false;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;

  auto design = ducode::Design::parse_json(json_file);
  design.write_verilog(exported_design, params);
  REQUIRE(boost::filesystem::exists(exported_design));

  auto data_reference = ducode::simulate_design(exported_design, reference_tb);

  const VCDScope* root_scope = &ducode::find_root_scope(data_reference, design);
  ducode::VCDSignalsDataManager testbench_value_map(data_reference, root_scope);
  const ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));
  testbench.write_verilog(exported_testbench, params);

  auto data = ducode::simulate_design(exported_design, exported_testbench);

  ducode::DesignInstance instance = ducode::DesignInstance::create_instance(design);
  const VCDScope* root_scope_2 = &ducode::find_root_scope(data, design);
  ducode::VCDSignalsDataManager test_value_map(data, root_scope_2);
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));
  //instance.write_graphviz("SPI_Master.dot");

  // input output simulation should be enough for such a complex design.
  // in this testbench there are multiple naming mismatches between the reference_tb and the exported testbench due to the reference_tb having different names for the top module signals
  //so only check the top module signals

  const auto& dut_scope = data->get_scope("Ins");
  const auto& dut_scope_reference = data_reference->get_scope("uut");
  for (auto* scope_reference_signal: dut_scope_reference.signals) {
    auto signal_it = std::ranges::find_if(dut_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    if (signal_it != dut_scope.signals.end()) {
      spdlog::info("{}", scope_reference_signal->reference);
      CHECK(ducode::no_repetitions_compare(data_reference->get_signal_values(scope_reference_signal->hash), data->get_signal_values((*signal_it)->hash)));
    }
  }
}

TEST_CASE("Export SPI_Slave_tb", "[export][.]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "SPI_Slave" / "SPI_Slave.json";
  auto reference_tb = boost::filesystem::path{TESTFILES_DIR} / "SPI_Slave" / "SPI_Slave_tb.v";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  REQUIRE(created);
  auto exported_design = temp_dir / "ift_SPI_Slave.v";
  auto exported_testbench = temp_dir / "ift_SPI_Slave_tb.v";

  nlohmann::json params;
  params["ift"] = false;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;

  auto design = ducode::Design::parse_json(json_file);
  design.write_verilog(exported_design, params);
  REQUIRE(boost::filesystem::exists(exported_design));

  auto data_reference = ducode::simulate_design(exported_design, reference_tb);

  const VCDScope* root_scope = &ducode::find_root_scope(data_reference, design);
  ducode::VCDSignalsDataManager testbench_value_map(data_reference, root_scope);
  const ducode::Testbench testbench(design, std::make_unique<ducode::VCDSignalsDataManager>(testbench_value_map));
  testbench.write_verilog(exported_testbench, params);

  auto data = ducode::simulate_design(exported_design, exported_testbench);

  ducode::DesignInstance instance = ducode::DesignInstance::create_instance(design);
  root_scope = &ducode::find_root_scope(data, design);
  ducode::VCDSignalsDataManager test_value_map(data, root_scope);
  instance.add_signal_tag_map(std::make_unique<ducode::VCDSignalsDataManager>(test_value_map));
  //instance.write_graphviz("SPI_Slave.dot");

  // input output simulation should be enough for such a complex design.
  const auto& outer_scope = data->get_scope("SPI_Slave_tb");
  const auto& outer_scope_reference = data_reference->get_scope("SPI_Slave_tb");
  for (auto* scope_reference_signal: outer_scope_reference.signals) {
    spdlog::info("{}", scope_reference_signal->reference);
    auto signal_it = std::ranges::find_if(outer_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    REQUIRE(signal_it != outer_scope.signals.end());
    CHECK(ducode::no_repetitions_compare(data_reference->get_signal_values(scope_reference_signal->hash), data->get_signal_values((*signal_it)->hash)));
  }

  const auto& dut_scope = data->get_scope("Ins");
  const auto& dut_scope_reference = data_reference->get_scope("Slave");
  for (auto* scope_reference_signal: dut_scope_reference.signals) {
    auto signal_it = std::ranges::find_if(dut_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    if (signal_it != dut_scope.signals.end()) {
      spdlog::info("{}", scope_reference_signal->reference);
      CHECK(ducode::no_repetitions_compare(data_reference->get_signal_values(scope_reference_signal->hash), data->get_signal_values((*signal_it)->hash)));
    }
  }
}
