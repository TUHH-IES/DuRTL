/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/design.hpp>
#include <ducode/port.hpp>
#include <ducode/utility/concepts.hpp>
#include <ducode/utility/parser_utility.hpp>

#include <fmt/core.h>
#include <fmt/ranges.h>
#include <nlohmann/json.hpp>
#include <range/v3/view/enumerate.hpp>
#include <vcd-parser/VCDFile.hpp>

#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace ducode {
/**
 * @brief The Testbench class represents a testbench for a design.
 * 
 * This class provides functionality to write Verilog testbenches and generate test vectors
 * based on VCD (Value Change Dump) files.
 */
class Testbench {
private:
  Design const* m_design;
  std::shared_ptr<VCDFile> m_vcd_data;

public:
  /**
   * @brief Constructs a Testbench object.
   *
   * This constructor initializes a Testbench object with the given design and VCD data.
   *
   * @param design The design object associated with the testbench.
   * @param vcd_data A shared pointer to the VCDFile object containing the VCD data.
   */
  Testbench(const Design& design, const std::shared_ptr<VCDFile>& vcd_data) : m_design(&design),
                                                                              m_vcd_data(vcd_data) {}

  template<filesystem_like T>
  /**
   * Writes the Verilog code for the given testbench and parameters.
   *
   * @param testbench The testbench to write Verilog code for.
   * @param params The parameters for the Verilog code generation.
   * @param dumpvars_lvl The level of dumpvars to include in the Verilog code (default is 0).
   */
  inline void write_verilog(const T& testbench, const nlohmann::json& params, int dumpvars_lvl = 0) const {
    return write_verilog(testbench.string(), params, dumpvars_lvl);
  }

  template<filesystem_like T>
  /**
   * Writes the Verilog code for the given testbench.
   * 
   * @param testbench The testbench to write Verilog code for.
   * @param tagged_ports A map of tagged ports.
   * @param params Additional parameters for writing Verilog code.
   * @param dumpvars_lvl The level of dumpvars.
   */
  inline void write_verilog(const T& testbench, std::unordered_map<std::string, std::string>& tagged_ports, const nlohmann::json& params, int dumpvars_lvl = 0) const {
    return write_verilog(testbench.string(), tagged_ports, params, dumpvars_lvl);
  }

  template<filesystem_like T>
  /**
   * Writes the Verilog code for the testbench, using the provided testbench file path, text input file path, and parameters.
   * 
   * @param testbench The file path of the testbench.
   * @param txt_input The file path of the text input.
   * @param params The JSON object containing the parameters.
   * @param dumpvars_lvl The level of dumpvars to be used (default is 0).
   */
  inline void write_verilog(const T& testbench, const T& txt_input, const nlohmann::json& params, int dumpvars_lvl = 0) const {
    return write_verilog(testbench.string(), txt_input.string(), params, dumpvars_lvl);
  }

  /**
   * Writes the Verilog testbench code to a file.
   *
   * @param testbench The path to the output testbench file.
   * @param params The JSON object containing the testbench parameters.
   * @param dumpvars_lvl The level of dumpvars to include in the testbench (default is 0).
   *
   * @throws std::runtime_error if the file cannot be opened.
   */
  void write_verilog(const std::string& testbench, const nlohmann::json& params, int dumpvars_lvl = 0) const {
    std::ofstream ofv_tb(testbench);
    if (!ofv_tb.is_open()) {
      throw std::runtime_error(fmt::format("Could not open file: {}", testbench));
    }
    ofv_tb << write_testbench("", std::unordered_map<std::string, std::string>(), params, dumpvars_lvl);
    ofv_tb.close();
  }

  /**
   * Writes the Verilog testbench code to the specified file.
   *
   * @param testbench The path to the output testbench file.
   * @param tagged_ports A map of tagged ports, where the key is the port name and the value is the tag.
   * @param params The JSON object containing the testbench parameters.
   * @param dumpvars_lvl The level of dumpvars to include in the testbench (default is 0).
   *
   * @throws std::runtime_error if the file cannot be opened.
   */
  void write_verilog(const std::string& testbench, std::unordered_map<std::string, std::string>& tagged_ports, const nlohmann::json& params, int dumpvars_lvl = 0) const {
    std::ofstream ofv_tb(testbench);
    if (!ofv_tb.is_open()) {
      throw std::runtime_error(fmt::format("Could not open file: {}", testbench));
    }
    ofv_tb << write_testbench("", tagged_ports, params, dumpvars_lvl);
    ofv_tb.close();
  }

  /**
   * Writes a Verilog testbench file.
   *
   * This function takes in the path to the testbench file, the path to the input file,
   * a JSON object containing parameters, and an optional level for dumping variables.
   * It creates a testbench file and writes the generated testbench code into it.
   *
   * @param testbench The path to the testbench file.
   * @param txt_input The path to the input file.
   * @param params A JSON object containing parameters.
   * @param dumpvars_lvl An optional level for dumping variables (default is 0).
   *
   * @throws std::runtime_error if the testbench file cannot be opened.
   */
  void write_verilog(const std::string& testbench, const std::string& txt_input, const nlohmann::json& params, int dumpvars_lvl = 0) const {
    std::ofstream ofv_tb(testbench);
    if (!ofv_tb.is_open()) {
      throw std::runtime_error(fmt::format("Could not open file: {}", testbench));
    }
    ofv_tb << write_testbench(txt_input, std::unordered_map<std::string, std::string>(), params, dumpvars_lvl);
    ofv_tb.close();
  }

protected:
  /**
   * Writes the testbench inputs for the given top module.
   *
   * @param top_module The top module for which the testbench inputs are generated.
   * @return A string containing the testbench inputs.
   */
  [[nodiscard]] std::string write_testbench_inputs(const Module& top_module) const {
    std::vector<std::string> input_names = top_module.get_input_names();

    std::stringstream result;

    VCDTime currTime = 0;
    VCDTime delta = 0;

    std::unordered_map<std::string, VCDSignalValues> signal_values;
    for (const VCDScope* scope: m_vcd_data->get_scopes().at(0).children) {
      for (const VCDSignal* signal: scope->signals) {
        if (std::ranges::find(input_names, signal->reference) != input_names.end()) {
          signal_values.emplace(signal->hash, m_vcd_data->get_signal_values(signal->hash));
        }
      }
    }

    auto get_signal_value_at = [&](const VCDSignalHash& hash, VCDTime time) -> const VCDValue& {
      auto find = signal_values.find(hash);
      if (find == signal_values.end()) {
        throw std::runtime_error("Signal not found");
      }
      auto& vals = find->second;
      if (vals.empty()) {
        throw std::runtime_error("Empty signal");
      }

      auto erase_until = vals.begin();
      bool found = false;

      for (auto it = vals.begin(); it != vals.end(); ++it) {
        if (it->time <= time) {
          erase_until = it;
          found = true;
        } else {
          break;
        }
      }

      if (!found) {
        throw std::runtime_error("Element not found");
      }

      // avoid O(n^2) performance for large sequential scans
      vals.erase(vals.begin(), erase_until);

      return erase_until->value;
    };

    result << "initial begin \n";

    for (const auto& time: m_vcd_data->get_timestamps()) {
      delta = time - currTime;
      currTime = time;
      const auto loop_max = 5;
      result << fmt::format("#{};\n", delta);

      for (const VCDScope* scope: m_vcd_data->get_scopes().at(0).children) {
        for (const VCDSignal* signal: scope->signals) {
          if (std::ranges::find(input_names, signal->reference) != input_names.end()) {
            auto bitValues = ducode::bit_to_vector(get_signal_value_at(signal->hash, time));
            result << fmt::format("{} = {}'b", signal->reference, signal->size);
            if (bitValues.size() == 1 && bitValues[0] == VCDBit::VCD_X) {
              for (uint64_t i = 0; i < (static_cast<uint64_t>(signal->size) - bitValues.size()); i++) {
                result << "X";
              }
            } else {
              for (uint64_t i = 0; i < (static_cast<uint64_t>(signal->size) - bitValues.size()); i++) {
                result << "0";
              }
            }
            for (const auto& bit: bitValues) {
              result << VCDValue::VCDBit2Char(bit);
            }
            result << ";\n";
          }
        }
      }
    }
    result << "$finish;\n";
    result << "end\n\n";
    return result.str();
  }

  /**
   * @brief Writes the internals of the testbench.
   *
   * This function generates the verilog code for the internals of the testbench, including the instantiation of the design top module,
   * the creation of verilog variables for reading in tags from an external file, and the verilog code for reading in a text file containing the tags.
   * It also handles the assignment of tags to ports and the generation of testbench inputs.
   *
   * @param top_module The design top module.
   * @param txtInputFile The path to the text file containing the tags.
   * @param tagged_ports A map of tagged ports.
   * @param params Additional parameters for the testbench.
   * @return A string containing the generated verilog code for the testbench internals.
   */
  [[nodiscard]] std::string write_testbench_internals(const Module& top_module, const std::string& txtInputFile, std::unordered_map<std::string, std::string>& tagged_ports, const nlohmann::json& params) const {
    Expects(txtInputFile.empty() || tagged_ports.empty());

    const bool ift = params["ift"].get<bool>();

    std::stringstream result;

    result << "integer i;\n";
    result << "integer t;\n\n";

    auto count_file_lines = [](const std::string& file) {
      std::size_t numLines = 0;
      std::ifstream in(file);
      std::string unused;
      while (std::getline(in, unused)) {
        if (!unused.empty()) {
          ++numLines;
        }
      }
      return numLines;
    };

    auto numLines = count_file_lines(txtInputFile);

    // creating verilog variables for reading in the tags from external file
    if (numLines > 0) {
      const std::size_t read_simulation_data_lines = (tag_size * top_module.get_input_names().size()) + tag_size;
      result << fmt::format("reg[{}:0] ", ((read_simulation_data_lines - 1)));
      if (numLines <= 1) {
        result << "read_simulation_data [0:0];\n";
      } else {
        result << fmt::format("read_simulation_data [0:{}];\n", numLines - 1);
      }
    }
    result << "\n\n";

    // instantiation of the design top module
    const auto input_names = top_module.get_input_names();
    const auto output_names = top_module.get_output_names();

    std::vector<std::string> port_assignments;
    port_assignments.reserve(input_names.size() * 2 + output_names.size() * 2);

    for (const auto& element: ranges::views::concat(input_names, output_names)) {
      port_assignments.emplace_back(fmt::format(".{}({})", legalize_identifier(element), legalize_identifier(element)));
      if (ift) {
        port_assignments.emplace_back(fmt::format(".{}_t({}_t)", legalize_identifier(element), legalize_identifier(element)));
      }
    }

    result << fmt::format("{}  Ins({});\n\n", top_module.get_name(), boost::join(port_assignments, ", "));

    // verilog code for reading in text file containing the tags
    if (ift) {
      if (numLines > 0) {
        result << "initial begin \n";
        for (const auto& input_name: input_names) {
          result << fmt::format("{}{} = {}'b00000000000000000000000000000000;\n", legalize_identifier(input_name), ift_tag, tag_size);
        }
        result << fmt::format("\t$readmemb({}, read_simulation_data);\n", "`INPUT_FILE_NAME");

        result << fmt::format("\t\tfor(i = 0; i<={}; i++) begin\n", numLines - 1);

        std::vector<std::string> read_ports;
        read_ports.reserve(input_names.size());
        for (const auto& input_name: input_names) {
          read_ports.emplace_back(fmt::format("{}{}", legalize_identifier(input_name), ift_tag));
        }

        result << fmt::format("\t\t{{{}, t}} = read_simulation_data[i];\n", boost::join(read_ports, ", "));
        result << "\t\t#t;\n";// time test to correspond to vcd file.  May have to change later
        result << "\tend\n";
        result << "end\n\n";
      }
    }

    if (!tagged_ports.empty()) {
      result << fmt::format("reg [{}:0] tag = 1;\n", tag_size);
      for (uint64_t i = 0; i < tagged_ports.size(); i++) {
        result << fmt::format("wire [{}:0] tag_{};\n", tag_size, i);
        result << fmt::format("wire s{};\n", i);
      }
      result << fmt::format("wire [{}:0] tag_{};\n", tag_size, tagged_ports.size());

      result << fmt::format("assign tag_0 =  (^tag === 1'bx) ? 32'b00000000000000000000000000000001 :  tag;\n");
      for (const auto& [index, port]: ranges::views::enumerate(tagged_ports)) {
        result << fmt::format("assign s{} = {} != {};\n", index, top_module.get_port(port.first).m_name, top_module.get_port(port.second).m_name);
        result << fmt::format("assign {}{} = s{} ? tag_{} : 32'b0;\n", top_module.get_port(port.first).m_name, ift_tag, index, index);
        result << fmt::format("assign tag_{} = s{} ? tag_{} << 1 : tag_{};\n", index + 1, index, index, index);
      }

      for (const auto& port: top_module.get_ports()) {
        if (!tagged_ports.contains(port.m_name) && port.m_direction == ducode::Port::Direction::input) {
          result << fmt::format("assign {}{} = 0;\n", port.m_name, ift_tag);
        }
      }

      result << fmt::format("always @(posedge clk) begin\n");
      result << fmt::format("    tag = tag_{};\n", tagged_ports.size());
      result << fmt::format("end\n");
    }

    result << write_testbench_inputs(top_module);
    return result.str();
  }

  /**
   * @brief Generates a testbench module for a given design.
   * 
   * This function generates a testbench module based on the provided design. The testbench module includes the necessary
   * declarations, internal logic, and initialization code for the testbench. The generated testbench module is returned
   * as a string.
   * 
   * @param txtInputFile The input file for the testbench.
   * @param tagged_ports A map of tagged ports.
   * @param params Additional parameters for the testbench.
   * @param dumpvars_lvl The level of dumpvars.
   * @return The generated testbench module as a string.
   * 
   * @throws std::runtime_error if the design does not contain a top module or if the testbench contains more than one
   * instantiated design.
   */
  [[nodiscard]] std::string write_testbench(const std::string& txtInputFile, std::unordered_map<std::string, std::string> tagged_ports, const nlohmann::json& params, int dumpvars_lvl) const {
    Expects(params.contains("ift"));

    const bool ift = params["ift"].get<bool>();

    auto tb_module = std::ranges::find_if(m_design->get_modules(), [&](const auto& mod) { return mod.get_name() == m_design->get_top_module_name(); });
    if (tb_module == m_design->get_modules().end()) {
      throw std::runtime_error("This design does not contain a top module");
    }

    auto top_module = tb_module;
    if (tb_module->get_ports().empty()) {// this is a testbench

      if (tb_module->get_cells().size() != 1) {
        throw std::runtime_error("Here we assume that the testbench contains only 1 instantiated design");
      }

      top_module = std::ranges::find_if(m_design->get_modules(), [&](const auto& mod) { return mod.get_name() == tb_module->get_cells().begin()->get_type(); });
    }

    std::stringstream testbench;

    // header section

    testbench << fmt::format("module {}_tb;\n", top_module->get_name());

    for (const auto& port: top_module->get_ports()) {
      std::string width_string = fmt::format(" [{}:0] ", port.m_bits.size() - 1);
      std::string type_string = "wire";
      if (port.m_direction == ducode::Port::Direction::input) {
        type_string = "reg";
      }
      testbench << fmt::format("{} {}{};\n", type_string, width_string, legalize_identifier(port.m_name));
      if (ift) {
        if (tagged_ports.empty()) {
          testbench << fmt::format("{} [{}:0] {}{}{};\n", type_string, (tag_size - 1), legalize_identifier(port.m_name), ift_tag, type_string == "reg" ? " = 0" : "");
        } else {
          testbench << fmt::format("{} [{}:0] {}{};\n", "wire", (tag_size - 1), legalize_identifier(port.m_name), ift_tag);
        }
      }
    }

    // internals section
    testbench << write_testbench_internals(*top_module, txtInputFile, tagged_ports, params);

    // footer section
    testbench << "initial\nbegin\n";
    testbench << "\t$dumpfile(`DUMP_FILE_NAME);\n";
    testbench << fmt::format("\t$dumpvars({});\n", dumpvars_lvl);
    testbench << "end\n\n\nendmodule";
    return testbench.str();
  }
};
}// namespace ducode