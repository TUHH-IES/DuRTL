/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/cell.hpp>
#include <ducode/design.hpp>
#include <ducode/ift_analysis.hpp>
#include <ducode/sim.hpp>
#include <ducode/testbench.hpp>
#include <ducode/utility/VCD_comparisons.hpp>
#include <ducode/utility/graph.hpp>
#include <ducode/utility/iverilog_wrapper.hpp>

#include <catch2/catch_all.hpp>
#include <vcd-parser/VCDComparisons.hpp>

#include <string>

/**
 * @brief Performs a simulation test on a design.
 * 
 * This function takes a path to a JSON file and performs a simulation test on the design specified in the file.
 * It creates a temporary directory, exports the design to Verilog, and simulates the exported Verilog design using a reference testbench.
 * The simulation results are returned as data.
 * 
 * @param path The path to the JSON file containing the design.
 * @return The simulation data.
 */
auto test_simulation(const std::string& path) {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / (path + ".json");
  auto reference_tb = boost::filesystem::path{TESTFILES_DIR} / (path + "_tb.v");
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  REQUIRE(created);
  auto exported_design = temp_dir / "exported_verilog.v";

  nlohmann::json params;
  params["ift"] = false;

  auto design = ducode::Design::parse_json(json_file.string());
  design.write_verilog(exported_design, params);

  auto data = ducode::simulate_design(exported_design, reference_tb);

  return data;
}

TEST_CASE("Export multiple modules", "[export]") {
  auto data = test_simulation("export/moduleinstanceTest");

  auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / "export" / "moduleinstanceTest.vcd";

  spdlog::debug("Load reference VCD FILE: {}", vcd_reference_file.string());
  REQUIRE(boost::filesystem::exists(vcd_reference_file));

  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_reference_file.string());

  spdlog::debug("Comparing vcd-files from original design simulation {} and exported design simulation", vcd_reference_file.string());

  const auto& inner_scope = data->get_scope("modins");
  const auto& inner_scope_reference = vcd_reference->get_scope("modins");
  for (auto* scope_reference_signal: inner_scope_reference.signals) {
    auto signal_it = std::ranges::find_if(inner_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    CHECK(signal_it != inner_scope.signals.end());
    CHECK(vcd_reference->get_signal_values(scope_reference_signal->hash) == data->get_signal_values((*signal_it)->hash));
  }

  const auto& outer_scope = data->get_scope("ins");
  const auto& outer_scope_reference = vcd_reference->get_scope("ins");
  for (auto* scope_reference_signal: outer_scope_reference.signals) {
    auto signal_it = std::ranges::find_if(outer_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    CHECK(signal_it != outer_scope.signals.end());
    CHECK(vcd_reference->get_signal_values(scope_reference_signal->hash) == data->get_signal_values((*signal_it)->hash));
  }
}

TEST_CASE("Export split signals", "[export]") {
  auto data = test_simulation("export/split/split");

  auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / "export" / "split" / "split.vcd";

  spdlog::debug("Load reference VCD FILE: {}", vcd_reference_file.string());
  REQUIRE(boost::filesystem::exists(vcd_reference_file));

  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_reference_file.string());

  spdlog::debug("Comparing vcd-files from original design simulation {} and exported design simulation", vcd_reference_file.string());

  const auto& inner_scope = data->get_scope("a1");
  const auto& inner_scope_reference = vcd_reference->get_scope("a1");
  for (auto* scope_reference_signal: inner_scope_reference.signals) {
    auto signal_it = std::ranges::find_if(inner_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    CHECK(signal_it != inner_scope.signals.end());
    CHECK(vcd_reference->get_signal_values(scope_reference_signal->hash) == data->get_signal_values((*signal_it)->hash));
  }

  const auto& outer_scope = data->get_scope("test");
  const auto& outer_scope_reference = vcd_reference->get_scope("test");
  for (auto* scope_reference_signal: outer_scope_reference.signals) {
    auto signal_it = std::ranges::find_if(outer_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    CHECK(signal_it != outer_scope.signals.end());
    CHECK(vcd_reference->get_signal_values(scope_reference_signal->hash) == data->get_signal_values((*signal_it)->hash));
  }
}

TEST_CASE("Export y86", "[export]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "y86" / "y86.json";
  auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / "y86" / "y86_gate_level.vcd";
  auto reference_tb = boost::filesystem::path{TESTFILES_DIR} / "y86" / "y86_tb.v";
  auto reference_input = boost::filesystem::path{TESTFILES_DIR} / "y86" / "y86.txt";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  REQUIRE(created);
  auto exported_design = temp_dir / "y86TestIFT.v";

  nlohmann::json params;
  params["ift"] = false;

  auto design = ducode::Design::parse_json(json_file.string());
  design.write_verilog(exported_design, params);
  REQUIRE(boost::filesystem::exists(exported_design));

  for (const ducode::Module& module: design.get_modules()) {
    CHECK_FALSE(ducode::contains_loops(module.get_graph()));
    module.write_graphviz(fmt::format("{}.dot", module.get_name()));
  }

  auto data = ducode::simulate_design(exported_design, reference_tb, reference_input);

  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_reference_file.string());

  // input output simulation should be enough for such a complex design.
  const auto& outer_scope = data->get_scope("main");
  const auto& outer_scope_reference = vcd_reference->get_scope("main");
  for (auto* scope_reference_signal: outer_scope_reference.signals) {
    auto signal_it = std::ranges::find_if(outer_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    CHECK(signal_it != outer_scope.signals.end());
    CHECK(ducode::no_repetitions_compare(vcd_reference->get_signal_values(scope_reference_signal->hash), data->get_signal_values((*signal_it)->hash)));
  }

  const auto& dut_scope = data->get_scope("dut");
  const auto& dut_scope_reference = vcd_reference->get_scope("dut");
  for (auto* scope_reference_signal: dut_scope_reference.signals) {
    auto signal_it = std::ranges::find_if(dut_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    if (signal_it != dut_scope.signals.end()) {
      CHECK(ducode::no_repetitions_compare(vcd_reference->get_signal_values(scope_reference_signal->hash), data->get_signal_values((*signal_it)->hash)));
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

  auto design = ducode::Design::parse_json(json_file.string());
  design.write_verilog(exported_design, params);
  REQUIRE(boost::filesystem::exists(exported_design));

  auto data_reference = ducode::simulate_design(exported_design, reference_tb, reference_input);

  const ducode::Testbench testbench(design, data_reference);
  testbench.write_verilog(exported_testbench, params);

  auto data = ducode::simulate_design(exported_design, exported_testbench);

  ducode::DesignInstance instance = ducode::DesignInstance::create_instance(design);
  instance.add_vcd_data(data);
  instance.write_graphviz("y86.dot");

  // input output simulation should be enough for such a complex design.
  const auto& outer_scope = data->get_scope("cisc_cpu_p_tb");
  const auto& outer_scope_reference = data_reference->get_scope("main");
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

TEST_CASE("Export aes_128_tb", "[export][.]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "aes_128" / "aes_128.json";
  auto reference_tb = boost::filesystem::path{TESTFILES_DIR} / "aes_128" / "aes_128_tb.v";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  REQUIRE(created);
  auto exported_design = temp_dir / "ift_aes_128.v";
  auto exported_testbench = temp_dir / "ift_aes_128_tb.v";

  nlohmann::json params;
  params["ift"] = false;

  auto design = ducode::Design::parse_json(json_file.string());
  design.write_verilog(exported_design, params);
  REQUIRE(boost::filesystem::exists(exported_design));

  auto data_reference = ducode::simulate_design(exported_design, reference_tb);

  const ducode::Testbench testbench(design, data_reference);
  testbench.write_verilog(exported_testbench, params);

  auto data = ducode::simulate_design(exported_design, exported_testbench);

  ducode::DesignInstance instance = ducode::DesignInstance::create_instance(design);
  instance.add_vcd_data(data);
  instance.write_graphviz("aes_128.dot");

  // input output simulation should be enough for such a complex design.
  const auto& outer_scope = data->get_scope("aes_128_tb");
  const auto& outer_scope_reference = data_reference->get_scope("test_aes_128");
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

  auto design = ducode::Design::parse_json(json_file.string());
  design.write_verilog(exported_design, params);
  REQUIRE(boost::filesystem::exists(exported_design));

  auto data_reference = ducode::simulate_design(exported_design, reference_tb);

  const ducode::Testbench testbench(design, data_reference);
  testbench.write_verilog(exported_testbench, params);

  auto data = ducode::simulate_design(exported_design, exported_testbench);

  ducode::DesignInstance instance = ducode::DesignInstance::create_instance(design);
  instance.add_vcd_data(data);
  instance.write_graphviz("SPI_Master.dot");

  // input output simulation should be enough for such a complex design.
  const auto& outer_scope = data->get_scope("SPI_Master_tb");
  const auto& outer_scope_reference = data_reference->get_scope("SPI_MasterTB");
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

  auto design = ducode::Design::parse_json(json_file.string());
  design.write_verilog(exported_design, params);
  REQUIRE(boost::filesystem::exists(exported_design));

  auto data_reference = ducode::simulate_design(exported_design, reference_tb);

  const ducode::Testbench testbench(design, data_reference);
  testbench.write_verilog(exported_testbench, params);

  auto data = ducode::simulate_design(exported_design, exported_testbench);

  ducode::DesignInstance instance = ducode::DesignInstance::create_instance(design);
  instance.add_vcd_data(data);
  instance.write_graphviz("SPI_Slave.dot");

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


TEST_CASE("Export picorv32", "[export]") {
  auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / "picorv32" / "picorv32_gate_level.vcd";

  auto data = test_simulation("picorv32/picorv32");

  REQUIRE(boost::filesystem::exists(vcd_reference_file));

  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_reference_file.string());

  // input output simulation should be enough for such a complex design.
  const auto& outer_scope = data->get_scope("testbench");
  const auto& outer_scope_reference = vcd_reference->get_scope("testbench");
  for (auto* scope_reference_signal: outer_scope_reference.signals) {
    spdlog::info(scope_reference_signal->reference);
    auto signal_it = std::ranges::find_if(outer_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    CHECK(signal_it != outer_scope.signals.end());
    CHECK(vcd_reference->get_signal_values(scope_reference_signal->hash) == data->get_signal_values((*signal_it)->hash));
  }
}

TEST_CASE("export spi master", "[export]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "SPI_Master" / "SPI_Master.json";
  auto vcd_file = boost::filesystem::path{TESTFILES_DIR} / "SPI_Master" / "SPI_Master_exported.vcd";
  auto design = ducode::Design::parse_json(json_file.string());
  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_file.string());

  auto vcd_data = test_simulation("SPI_Master/SPI_Master");

  const auto& outer_scope = vcd_data->get_scope("uut");
  const auto& outer_scope_reference = vcd_reference->get_scope("uut");
  for (const auto& port: design.get_module("SPI_Master").get_ports()) {

    auto lambda = [&](const auto& signal) { return signal->reference == port.m_name; };
    auto hash_iter = std::ranges::find_if(outer_scope.signals, lambda);
    auto hash_reference_iter = std::ranges::find_if(outer_scope_reference.signals, lambda);

    const auto& signal_values = vcd_data->get_signal_values((*hash_iter)->hash);
    const auto& signal_values_reference = vcd_reference->get_signal_values((*hash_reference_iter)->hash);
    CHECK(signal_values == signal_values_reference);
  }
}

TEST_CASE("export multiple_nets", "[export]") {
  const std::string design_name = "multiple_nets";
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / design_name / (design_name + ".json");
  auto vcd_file = boost::filesystem::path{TESTFILES_DIR} / design_name / "output.vcd";
  auto tb_file = boost::filesystem::path{TESTFILES_DIR} / design_name / (design_name + "_tb.v");
  auto verilog_file = boost::filesystem::path{TESTFILES_DIR} / design_name / (design_name + ".v");
  auto design = ducode::Design::parse_json(json_file.string());
  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_file.string());
  REQUIRE(vcd_reference != nullptr);

  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  REQUIRE(created);

  nlohmann::json params;
  params["ift"] = false;
  auto exported_design = temp_dir / "test.v";
  design.write_verilog(exported_design, params);
  REQUIRE(boost::filesystem::exists(exported_design));

  auto vcd_data = ducode::simulate_design(exported_design, tb_file);

  const auto& outer_scope = vcd_data->get_scope("dut");
  const auto& outer_scope_reference = vcd_reference->get_scope("dut");

  for (const auto& net: design.get_module(design_name).get_nets()) {
    if (net.is_hidden()) {
      continue;
    }
    auto lambda = [&](const auto& signal) { return signal->reference == net.get_name(); };
    auto hash_iter = std::ranges::find_if(outer_scope.signals, lambda);
    auto hash_reference_iter = std::ranges::find_if(outer_scope_reference.signals, lambda);
    REQUIRE(hash_iter != outer_scope.signals.end());
    REQUIRE(hash_reference_iter != outer_scope_reference.signals.end());

    const auto& signal_values = vcd_data->get_signal_values((*hash_iter)->hash);
    const auto& signal_values_reference = vcd_reference->get_signal_values((*hash_reference_iter)->hash);
    CHECK(signal_values == signal_values_reference);
  }
}

TEST_CASE("export simple_mem", "[export]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "simple_mem" / "simple_mem.json";
  auto vcd_file = boost::filesystem::path{TESTFILES_DIR} / "simple_mem" / "simple_mem.vcd";
  auto design = ducode::Design::parse_json(json_file.string());
  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_file.string());

  auto vcd_data = test_simulation("simple_mem/simple_mem");

  const auto& outer_scope = vcd_data->get_scope("simple_mem_tb");
  const auto& outer_scope_reference = vcd_reference->get_scope("simple_mem_tb");
  for (auto* scope_reference_signal: outer_scope_reference.signals) {
    spdlog::info("{}", scope_reference_signal->reference);
    auto signal_it = std::ranges::find_if(outer_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    REQUIRE(signal_it != outer_scope.signals.end());
    CHECK(vcd_reference->get_signal_values(scope_reference_signal->hash) == vcd_data->get_signal_values((*signal_it)->hash));
  }
}

TEST_CASE("export eth_wishbone", "[export]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "eth_wishbone" / "eth_wishbone.json";
  auto vcd_file = boost::filesystem::path{TESTFILES_DIR} / "eth_wishbone" / "eth_wishbone.vcd";
  auto design = ducode::Design::parse_json(json_file.string());
  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_file.string());

  auto vcd_data = test_simulation("eth_wishbone/eth_wishbone");

  spdlog::info("dut");
  const auto& inner_scope = vcd_data->get_scope("dut");
  const auto& inner_scope_reference = vcd_reference->get_scope("dut");
  for (auto* scope_reference_signal: inner_scope_reference.signals) {
    spdlog::info("{}", scope_reference_signal->reference);
    auto signal_it = std::ranges::find_if(inner_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    if (signal_it != inner_scope.signals.end()) {
      CHECK(vcd_reference->get_signal_values(scope_reference_signal->hash) == vcd_data->get_signal_values((*signal_it)->hash));
    }
  }

  spdlog::info("eth_wishbone_tb");
  const auto& outer_scope = vcd_data->get_scope("eth_wishbone_tb");
  const auto& outer_scope_reference = vcd_reference->get_scope("eth_wishbone_tb");
  for (auto* scope_reference_signal: outer_scope_reference.signals) {
    spdlog::info("{}", scope_reference_signal->reference);
    auto signal_it = std::ranges::find_if(outer_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    REQUIRE(signal_it != outer_scope.signals.end());
    if (scope_reference_signal->type != VCDVarType::VCD_VAR_PARAMETER) {
      CHECK(vcd_reference->get_signal_values(scope_reference_signal->hash) == vcd_data->get_signal_values((*signal_it)->hash));
    }
  }
}

TEST_CASE("export yadmc_sdram16", "[export]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "yadmc_sdram16" / "yadmc_sdram16.json";
  auto vcd_file = boost::filesystem::path{TESTFILES_DIR} / "yadmc_sdram16" / "yadmc_sdram16.vcd";
  auto design = ducode::Design::parse_json(json_file.string());
  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_file.string());

  auto vcd_data = test_simulation("yadmc_sdram16/yadmc_sdram16");

  spdlog::info("dut");
  const auto& inner_scope = vcd_data->get_scope("dut");
  const auto& inner_scope_reference = vcd_reference->get_scope("dut");
  for (auto* scope_reference_signal: inner_scope_reference.signals) {
    auto signal_it = std::ranges::find_if(inner_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    if (signal_it != inner_scope.signals.end()) {
      spdlog::info("{}", scope_reference_signal->reference);
      CHECK(vcd_reference->get_signal_values(scope_reference_signal->hash) == vcd_data->get_signal_values((*signal_it)->hash));
    }
  }

  spdlog::info("yadmc_sdram16_tb");
  const auto& outer_scope = vcd_data->get_scope("yadmc_sdram16_tb");
  const auto& outer_scope_reference = vcd_reference->get_scope("yadmc_sdram16_tb");
  for (auto* scope_reference_signal: outer_scope_reference.signals) {
    auto signal_it = std::ranges::find_if(outer_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    REQUIRE(signal_it != outer_scope.signals.end());
    if (scope_reference_signal->type != VCDVarType::VCD_VAR_PARAMETER) {
      spdlog::info("{}", scope_reference_signal->reference);
      CHECK(vcd_reference->get_signal_values(scope_reference_signal->hash) == vcd_data->get_signal_values((*signal_it)->hash));
    }
  }
}

TEST_CASE("export aes_128_mem", "[export]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "aes_128_mem" / "aes_128_mem.json";
  auto vcd_file = boost::filesystem::path{TESTFILES_DIR} / "aes_128_mem" / "aes_128_mem.vcd";
  auto design = ducode::Design::parse_json(json_file.string());
  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_file.string());

  auto vcd_data = test_simulation("aes_128_mem/aes_128_mem");

  spdlog::info("dut");
  const auto& inner_scope = vcd_data->get_scope("dut");
  const auto& inner_scope_reference = vcd_reference->get_scope("dut");
  for (auto* scope_reference_signal: inner_scope_reference.signals) {
    auto signal_it = std::ranges::find_if(inner_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    if (signal_it != inner_scope.signals.end()) {
      spdlog::info("{}", scope_reference_signal->reference);
      CHECK(vcd_reference->get_signal_values(scope_reference_signal->hash) == vcd_data->get_signal_values((*signal_it)->hash));
    }
  }

  spdlog::info("aes_128_mem_tb");
  const auto& outer_scope = vcd_data->get_scope("aes_128_mem_tb");
  const auto& outer_scope_reference = vcd_reference->get_scope("aes_128_mem_tb");
  for (auto* scope_reference_signal: outer_scope_reference.signals) {
    auto signal_it = std::ranges::find_if(outer_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    REQUIRE(signal_it != outer_scope.signals.end());
    if (scope_reference_signal->type != VCDVarType::VCD_VAR_PARAMETER) {
      spdlog::info("{}", scope_reference_signal->reference);
      CHECK(vcd_reference->get_signal_values(scope_reference_signal->hash) == vcd_data->get_signal_values((*signal_it)->hash));
    }
  }
}

TEST_CASE("export eth_fifo", "[export]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "eth_fifo" / "eth_fifo.json";
  auto vcd_file = boost::filesystem::path{TESTFILES_DIR} / "eth_fifo" / "eth_fifo.vcd";
  auto design = ducode::Design::parse_json(json_file.string());
  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_file.string());

  auto vcd_data = test_simulation("eth_fifo/eth_fifo");

  spdlog::info("dut");
  const auto& inner_scope = vcd_data->get_scope("dut");
  const auto& inner_scope_reference = vcd_reference->get_scope("dut");
  for (auto* scope_reference_signal: inner_scope_reference.signals) {
    spdlog::info("{}", scope_reference_signal->reference);
    auto signal_it = std::ranges::find_if(inner_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    if (signal_it != inner_scope.signals.end()) {
      CHECK(vcd_reference->get_signal_values(scope_reference_signal->hash) == vcd_data->get_signal_values((*signal_it)->hash));
    }
  }

  spdlog::info("eth_fifo_tb");
  const auto& outer_scope = vcd_data->get_scope("eth_fifo_tb");
  const auto& outer_scope_reference = vcd_reference->get_scope("eth_fifo_tb");
  for (auto* scope_reference_signal: outer_scope_reference.signals) {
    spdlog::info("{}", scope_reference_signal->reference);
    auto signal_it = std::ranges::find_if(outer_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    REQUIRE(signal_it != outer_scope.signals.end());
    if (scope_reference_signal->type != VCDVarType::VCD_VAR_PARAMETER) {
      CHECK(vcd_reference->get_signal_values(scope_reference_signal->hash) == vcd_data->get_signal_values((*signal_it)->hash));
    }
  }
}

TEST_CASE("export output_assignment", "[export]") {
  std::string design_name = "output_assignment";
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / design_name / (design_name + ".json");
  auto vcd_file = boost::filesystem::path{TESTFILES_DIR} / design_name / "output.vcd";
  auto tb_file = boost::filesystem::path{TESTFILES_DIR} / design_name / (design_name + "_tb.v");
  auto verilog_file = boost::filesystem::path{TESTFILES_DIR} / design_name / (design_name + ".v");
  auto design = ducode::Design::parse_json(json_file.string());
  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_file.string());
  REQUIRE(vcd_reference != nullptr);

  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  REQUIRE(created);

  nlohmann::json params;
  params["ift"] = false;
  auto exported_design = temp_dir / "test.v";
  design.write_verilog(exported_design, params);
  REQUIRE(boost::filesystem::exists(exported_design));

  for (const ducode::Module& module: design.get_modules()) {
    module.write_graphviz(module.get_name() + ".dot");
  }

  auto vcd_data = ducode::simulate_design(exported_design, tb_file);

  const auto& outer_scope = vcd_data->get_scope("dut");
  const auto& outer_scope_reference = vcd_reference->get_scope("dut");

  for (const auto& net: design.get_module(design_name).get_nets()) {
    if (net.is_hidden()) {
      continue;
    }
    auto lambda = [&](const auto& signal) { return signal->reference == net.get_name(); };
    auto hash_iter = std::ranges::find_if(outer_scope.signals, lambda);
    auto hash_reference_iter = std::ranges::find_if(outer_scope_reference.signals, lambda);
    REQUIRE(hash_iter != outer_scope.signals.end());
    REQUIRE(hash_reference_iter != outer_scope_reference.signals.end());

    const auto& signal_values = vcd_data->get_signal_values((*hash_iter)->hash);
    const auto& signal_values_reference = vcd_reference->get_signal_values((*hash_reference_iter)->hash);
    CHECK(signal_values == signal_values_reference);
  }
}

TEST_CASE("export spi slave", "[export]") {
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "SPI_Slave" / "SPI_Slave.json";
  auto vcd_file = boost::filesystem::path{TESTFILES_DIR} / "SPI_Slave" / "output_gate.vcd";
  auto tb_file = boost::filesystem::path{TESTFILES_DIR} / "SPI_Slave" / "SPI_Slave_tb.v";
  auto verilog_file = boost::filesystem::path{TESTFILES_DIR} / "SPI_Slave" / "SPI_Slave.v";
  auto design = ducode::Design::parse_json(json_file.string());
  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_file.string());
  REQUIRE(vcd_reference != nullptr);

  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  REQUIRE(created);

  nlohmann::json params;
  params["ift"] = false;
  auto exported_design = temp_dir / "test.v";
  design.write_verilog(exported_design, params);
  REQUIRE(boost::filesystem::exists(exported_design));

  auto vcd_data = ducode::simulate_design(exported_design, tb_file);

  const auto& outer_scope = vcd_data->get_scope("SPI_Slave_tb");
  const auto& outer_scope_reference = vcd_reference->get_scope("SPI_Slave_tb");
  for (const auto& port: design.get_module("SPI_Slave").get_ports()) {
    spdlog::info(port.m_name);
    auto lambda = [&](const auto& signal) { return signal->reference == port.m_name; };
    auto hash_iter = std::ranges::find_if(outer_scope.signals, lambda);
    auto hash_reference_iter = std::ranges::find_if(outer_scope_reference.signals, lambda);
    REQUIRE(hash_iter != outer_scope.signals.end());
    REQUIRE(hash_reference_iter != outer_scope_reference.signals.end());

    const auto& signal_values = vcd_data->get_signal_values((*hash_iter)->hash);
    const auto& signal_values_reference = vcd_reference->get_signal_values((*hash_reference_iter)->hash);
    CHECK(signal_values == signal_values_reference);
  }
}

TEST_CASE("Export xtea", "[export][.]") {
  auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / "xtea" / "xtea.vcd";

  auto data = test_simulation("xtea/xtea");

  spdlog::debug("Load reference VCD FILE: {}", vcd_reference_file.string());
  REQUIRE(boost::filesystem::exists(vcd_reference_file));

  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_reference_file.string());

  // input output simulation should be enough for such a complex design_file.
  const auto& outer_scope = data->get_scope("cipher_testbench");
  const auto& outer_scope_reference = vcd_reference->get_scope("cipher_testbench");
  for (auto* scope_reference_signal: outer_scope_reference.signals) {
    spdlog::info("{}", scope_reference_signal->reference);
    auto signal_it = std::ranges::find_if(outer_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    REQUIRE(signal_it != outer_scope.signals.end());
    CHECK(vcd_reference->get_signal_values(scope_reference_signal->hash) == data->get_signal_values((*signal_it)->hash));
  }
}

TEST_CASE("Export simple_inout", "[export]") {
  auto data = test_simulation("simple_inout/simple_inout");

  auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / "simple_inout" / "simple_inout.vcd";
  REQUIRE(boost::filesystem::exists(vcd_reference_file));

  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_reference_file.string());

  // input output simulation should be enough for such a complex design_file.
  const auto& outer_scope = data->get_scope("dut");
  const auto& outer_scope_reference = vcd_reference->get_scope("dut");
  for (auto* scope_reference_signal: outer_scope_reference.signals) {
    auto signal_it = std::ranges::find_if(outer_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    if (signal_it != outer_scope.signals.end()) {
      CHECK(vcd_reference->get_signal_values(scope_reference_signal->hash) == data->get_signal_values((*signal_it)->hash));
    }
  }
}

TEST_CASE("Export yadmc", "[export][.]") {
  auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / "yadmc" / "yadmc.vcd";

  auto data = test_simulation("yadmc/yadmc");

  REQUIRE(boost::filesystem::exists(vcd_reference_file));

  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_reference_file.string());

  // input output simulation should be enough for such a complex design_file.
  const auto& outer_scope = data->get_scope("yadmc_test");
  const auto& outer_scope_reference = vcd_reference->get_scope("yadmc_test");
  for (auto* scope_reference_signal: outer_scope_reference.signals) {
    auto signal_it = std::ranges::find_if(outer_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    CHECK(signal_it != outer_scope.signals.end());
    CHECK(vcd_reference->get_signal_values(scope_reference_signal->hash) == data->get_signal_values((*signal_it)->hash));
  }
}

TEST_CASE("Export usb2uart", "[export]") {
  auto path = boost::filesystem::path{TESTFILES_DIR} / "usb2uart";
  auto vcd_reference_file = path / "usb2uart.vcd";
  auto json_file = path / "usb2uart.json";
  auto reference_tb = path / "usb2uart_tb.v";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  REQUIRE(created);
  auto exported_design = temp_dir / "exported_verilog.v";

  nlohmann::json params;
  params["ift"] = false;

  auto design = ducode::Design::parse_json(json_file.string());
  design.write_verilog(exported_design, params);


  auto data = ducode::simulate_design(path, exported_design, reference_tb, boost::filesystem::path{});

  REQUIRE(boost::filesystem::exists(vcd_reference_file));

  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_reference_file.string());

  const auto& outer_scope = data->get_scope("usb2uart_tb");
  const auto& outer_scope_reference = vcd_reference->get_scope("usb2uart_tb");
  for (auto* scope_reference_signal: outer_scope_reference.signals) {
    auto signal_it = std::ranges::find_if(outer_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    REQUIRE(signal_it != outer_scope.signals.end());
    if (scope_reference_signal->type != VCDVarType::VCD_VAR_REAL) {
      spdlog::info("{}", scope_reference_signal->reference);
      CHECK(vcd_reference->get_signal_values(scope_reference_signal->hash) == data->get_signal_values((*signal_it)->hash));
    }
  }
}

TEST_CASE("Export yadmc_dpram read", "[export]") {
  auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / "yadmc_dpram" / "yadmc_dpram_read.vcd";
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "yadmc_dpram" / "yadmc_dpram.json";
  auto reference_tb = boost::filesystem::path{TESTFILES_DIR} / "yadmc_dpram" / "yadmc_dpram_read_tb.v";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  REQUIRE(created);
  auto exported_design = temp_dir / "exported_verilog.v";

  nlohmann::json params;
  params["ift"] = false;

  auto design = ducode::Design::parse_json(json_file.string());
  design.write_verilog(exported_design, params);

  auto data = ducode::simulate_design(exported_design, reference_tb);

  REQUIRE(boost::filesystem::exists(vcd_reference_file));

  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_reference_file.string());

  const auto& outer_scope = data->get_scope("dut");
  const auto& outer_scope_reference = vcd_reference->get_scope("dut");
  for (auto* scope_reference_signal: outer_scope_reference.signals) {
    auto signal_it = std::ranges::find_if(outer_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    if (signal_it != outer_scope.signals.end()) {
      spdlog::info("{}", scope_reference_signal->reference);
      CHECK(vcd_reference->get_signal_values(scope_reference_signal->hash) == data->get_signal_values((*signal_it)->hash));
    }
  }
}

TEST_CASE("Export yadmc_dpram write", "[export]") {
  auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / "yadmc_dpram" / "yadmc_dpram_write.vcd";
  auto json_file = boost::filesystem::path{TESTFILES_DIR} / "yadmc_dpram" / "yadmc_dpram.json";
  auto reference_tb = boost::filesystem::path{TESTFILES_DIR} / "yadmc_dpram" / "yadmc_dpram_write_tb.v";
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  REQUIRE(created);
  auto exported_design = temp_dir / "exported_verilog.v";

  nlohmann::json params;
  params["ift"] = false;

  auto design = ducode::Design::parse_json(json_file.string());
  design.write_verilog(exported_design, params);

  auto data = ducode::simulate_design(exported_design, reference_tb);

  REQUIRE(boost::filesystem::exists(vcd_reference_file));

  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_reference_file.string());

  const auto& outer_scope = data->get_scope("dut");
  const auto& outer_scope_reference = vcd_reference->get_scope("dut");
  for (auto* scope_reference_signal: outer_scope_reference.signals) {
    auto signal_it = std::ranges::find_if(outer_scope.signals, [&](auto signal) { return signal->reference == scope_reference_signal->reference; });
    if (signal_it != outer_scope.signals.end()) {
      spdlog::info("{}", scope_reference_signal->reference);
      CHECK(vcd_reference->get_signal_values(scope_reference_signal->hash) == data->get_signal_values((*signal_it)->hash));
    }
  }
}

TEST_CASE("export fft16", "[export]") {
  auto json_file = std::filesystem::path(TESTFILES_DIR) / "fft" / "fft16.json";
  auto vcd_file = boost::filesystem::path{TESTFILES_DIR} / "fft" / "fft16.vcd";

  auto design = ducode::Design::parse_json(json_file);
  VCDFileParser parser;
  auto vcd_reference = parser.parse_file(vcd_file.string());
  REQUIRE(vcd_reference != nullptr);

  for (const ducode::Module& module: design.get_modules()) {
    module.write_graphviz(module.get_name() + ".dot");
  }

  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  REQUIRE(created);

  nlohmann::json params;
  params["ift"] = false;

  auto exported_testbench = temp_dir / "testbench.v";
  auto exported_design = temp_dir / "test.v";
  design.write_verilog(exported_design, params);
  REQUIRE(boost::filesystem::exists(exported_design));

  const ducode::Testbench testbench(design, vcd_reference);
  testbench.write_verilog(exported_testbench, params);
  REQUIRE(boost::filesystem::exists(exported_testbench));

  auto vcd_data = ducode::simulate_design(exported_design, exported_testbench);

  const auto& outer_scope = vcd_data->get_scope("fft16_tb");
  const auto& outer_scope_reference = vcd_reference->get_scope("fft16_tb");
  for (const auto& port: design.get_module("fft16").get_ports()) {
    spdlog::info(port.m_name);
    auto lambda = [&](const auto& signal) { return signal->reference == port.m_name; };
    auto hash_iter = std::ranges::find_if(outer_scope.signals, lambda);
    auto hash_reference_iter = std::ranges::find_if(outer_scope_reference.signals, lambda);
    REQUIRE(hash_iter != outer_scope.signals.end());
    REQUIRE(hash_reference_iter != outer_scope_reference.signals.end());

    auto signal_values = vcd_data->get_signal_values((*hash_iter)->hash);
    const auto& signal_values_reference = vcd_reference->get_signal_values((*hash_reference_iter)->hash);

    for (auto& element: signal_values) {
      if (element.value.get_type() == VCDValueType::VECTOR && element.value.get_value_vector().size() != port.m_bits.size()) {
        auto new_vector = element.value.get_value_vector();
        if (element.value.get_value_vector().size() == 1) {
          for (auto i = 0ul; i < port.m_bits.size() - element.value.get_value_vector().size(); ++i) {
            new_vector.emplace_back(element.value.get_value_vector().at(0));
          }
        } else if (element.value.get_value_vector().size() > 1) {
          for (auto i = 0ul; i < port.m_bits.size() - element.value.get_value_vector().size(); ++i) {
            new_vector.emplace(new_vector.cbegin(), VCDBit::VCD_0);
          }
        }
        element.value = VCDValue(new_vector);
      }
    }

    CHECK(signal_values == signal_values_reference);
  }
}