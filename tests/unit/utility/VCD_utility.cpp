/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/utility/VCD_utility.hpp>
#include <ducode/utility/iverilog_wrapper.hpp>

#include <boost/filesystem/operations.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <fmt/core.h>
#include <nlohmann/json_fwd.hpp>
#include <spdlog/spdlog.h>

#include <cstdint>
#include <utility>

TEST_CASE("find_root_scope", "[VCD_utility]") {
  auto [design_folder, result] = GENERATE(
      std::pair{"counter/counter", "c1"},
      std::pair{"hierarchy/hierarchy", "t1"},
      std::pair{"constants/constants", "dut"},
      std::pair{"export/moduleinstanceTest", "ins"},
      std::pair{"export/split/split", "a1"},
      std::pair{"picorv32/picorv32", "uut"},
      std::pair{"y86/y86", "dut"},
      std::pair{"SPI_Master/SPI_Master", "uut"},
      std::pair{"multiple_nets/multiple_nets", "dut"},
      std::pair{"simple_mem/simple_mem", "dut"},
      std::pair{"eth_wishbone/eth_wishbone", "dut"},
      std::pair{"aes_128_mem/aes_128_mem", "dut"},
      std::pair{"yadmc_sdram16/yadmc_sdram16", "dut"},
      std::pair{"output_assignment/output_assignment", "dut"},
      std::pair{"eth_fifo/eth_fifo", "dut"},
      std::pair{"SPI_Slave/SPI_Slave", "Slave"},
      std::pair{"mips_16_core_top/mips_16_core_top", "uut"},
      std::pair{"yadmc_dpram/yadmc_dpram_write", "dut"},
      std::pair{"yadmc_dpram/yadmc_dpram_read", "dut"},
      std::pair{"usb2uart/usb2uart", "dut"},
      std::pair{"i2c_controller/i2c_controller", "master"},
      std::pair{"and/and", "d1"});


  spdlog::info("*********** Checking: {} **************", design_folder);

  auto json_file = boost::filesystem::path{TESTFILES_DIR} / fmt::format("{}.json", design_folder);
  auto reference_tb = boost::filesystem::path{TESTFILES_DIR} / fmt::format("{}_tb.v", design_folder);
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  REQUIRE(created);
  auto exported_design = temp_dir / "exported_verilog.v";

  nlohmann::json params;
  params["ift"] = false;
  const uint32_t tag_size = 32;
  params["tag_size"] = tag_size;

  auto design = ducode::Design::parse_json(json_file);

  design.write_verilog(exported_design, params);
  auto vcd_data = ducode::simulate_design(json_file.parent_path(), exported_design, reference_tb, {});

  auto scope = find_root_scope(vcd_data, design);

  CHECK(scope.name == result);
}