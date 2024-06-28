/* SPDX-License-Identifier: Apache-2.0 */

#include <common_definitions.hpp>
#include <ducode/design.hpp>
#include <ducode/testbench.hpp>
#include <ducode/utility/iverilog_wrapper.hpp>

#include <boost/filesystem/operations.hpp>
#include <catch2/catch_all.hpp>
#include <spdlog/spdlog.h>
#include <vcd-parser/VCDComparisons.hpp>
#include <vcd-parser/VCDFileParser.hpp>

#include <algorithm>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

TEST_CASE("testbench export for basic gates", "[testbench]") {
  nlohmann::json params;
  params["ift"] = false;
  params["apprx"] = false;

  std::vector<std::string> design_names = {"andGate", "orGate", "xorGate", "andVector", "mux", "logic_andGate", "logic_orGate", "LeCell", "GeCell", "EqCell", "LtCell", "GtCell", "reduceAndGate", "reduceOrGate", "reduceXorGate", "reduceXnorGate", "posGate"};

  for (const auto& design_name: design_names) {

    auto json_file = std::filesystem::path{TESTFILES_DIR} / "flattened_verilog" / (design_name + ".json");
    auto vcd_reference_file = boost::filesystem::path{TESTFILES_DIR} / "flattened_verilog" / (design_name + ".vcd");
    auto design_file = boost::filesystem::path{TESTFILES_DIR} / "flattened_verilog" / (design_name + ".v");
    auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
    auto created = boost::filesystem::create_directories(temp_dir);
    assert(created);
    auto tb_file = temp_dir / (design_name + "_tb.v");

    auto design = ducode::Design::parse_json(json_file.string());

    VCDFileParser parser;
    auto vcd_data = parser.parse_file(vcd_reference_file.string());

    const ducode::Testbench testbench(design, vcd_data);
    testbench.write_verilog(tb_file, params, 1);

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