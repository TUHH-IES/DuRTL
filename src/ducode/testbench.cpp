/* SPDX-License-Identifier: Apache-2.0 */

#include <ducode/ids.hpp>
#include <ducode/module.hpp>
#include <ducode/testbench.hpp>
#include <ducode/utility/legalize_identifier.hpp>
#include <ducode/utility/parser_utility.hpp>

#include "ducode/instantiation_graph_traits.hpp"
#include "ducode/utility/simulation.hpp"
#include <boost/algorithm/string/join.hpp>
#include <boost/dynamic_bitset/dynamic_bitset.hpp>
//#include <catch2/catch_test_macros.hpp>
#include <fmt/core.h>
#include <gsl/assert>
#include <gsl/narrow>
#include <nlohmann/json_fwd.hpp>
#include <range/v3/view/concat.hpp>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace ducode {

/**
   * Writes the testbench inputs for the given top module.
   *
   * @param top_module The top module for which the testbench inputs are generated.
   * @return A string containing the testbench inputs.
   */
[[nodiscard]] std::string Testbench::write_testbench_inputs() const {
  std::vector<SignalIdentifier> input_ids = m_design->get_input_signal_ids();
  std::stringstream result;

  SignalTime currTime = 0;
  SignalTime delta = 0;

  // get the signal values for the input signals
  std::unordered_map<SignalHash, SignalValues> signal_values = m_signal_tag_map->get_input_values(input_ids);

  // can stay here, because we only need the signal values for the input signals
  // get the signal value for a signal at a given time
  auto get_signal_value_at = [&](const SignalHash& hash, SignalTime time) -> const SignalValue& {
    auto find = signal_values.find(hash);
    if (find == signal_values.end()) {
      throw std::runtime_error("Signal not found");
    }
    auto& vals = find->second;
    if (vals.empty()) {
      throw std::runtime_error("Empty signal");
    }

    auto erase_until = vals.m_timed_signal_values.begin();
    bool found = false;

    for (auto it = vals.m_timed_signal_values.begin(); it != vals.m_timed_signal_values.end(); ++it) {
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
    vals.m_timed_signal_values.erase(vals.m_timed_signal_values.begin(), erase_until);

    return erase_until->value;
  };

  result << "initial begin \n";

  // write the input signals
  // get_timestamps => function in SignalValueMap
  for (const auto& time: m_signal_tag_map->get_timestamps()) {
    delta = time - currTime;
    currTime = time;
    result << fmt::format("#{};\n", delta);

    for (const SignalIdentifier& signal_id: input_ids) {
      SignalInfo signal = m_signal_tag_map->get_signal(signal_id);
      auto bitValues = ducode::bit_to_vector(get_signal_value_at(signal.hash, time));
      result << fmt::format("{} = {}'b", signal.reference, signal.size);
      if (bitValues.size() == 1 && bitValues[0] == SignalBit::BIT_X) {
        for (uint64_t i = 0; i < (static_cast<uint64_t>(signal.size) - bitValues.size()); i++) {
          result << "X";
        }
      } else {
        for (uint64_t i = 0; i < (static_cast<uint64_t>(signal.size) - bitValues.size()); i++) {
          result << "0";
        }
      }
      for (const auto& bit: bitValues) {
        result << SignalValue::signal_bit_to_char(bit);
      }
      result << ";\n";
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
   * @param tagged_ports A map of tagged ports.
   * @param params Additional parameters for the testbench.
   * @return A string containing the generated verilog code for the testbench internals.
   */
[[nodiscard]] std::string Testbench::write_testbench_internals(const nlohmann::json& params) const {
  Expects(params.contains("tag_size"));
  Expects(params.contains("ift"));

  const uint32_t tag_size = params["tag_size"].get<uint32_t>();
  const bool ift = params["ift"].get<bool>();

  std::stringstream result;

  result << "integer i;\n";
  result << "integer t;\n\n";

  uint32_t numLines = 0;
  if (ift && tagged_ports.empty()) {
    Expects(params.contains("timesteps_per_simulation_run"));
    numLines = params["timesteps_per_simulation_run"];

    // creating verilog variables for reading in the tags from external file
    if (numLines > 0) {
      const std::size_t read_simulation_data_lines = (tag_size * m_design->get_top_module().get_input_names().size()) + tag_size;
      result << fmt::format("reg [{}:0] ", ((read_simulation_data_lines - 1)));
      if (numLines <= 1) {
        result << "read_simulation_data [0:0];\n";
      } else {
        result << fmt::format("read_simulation_data [0:{}];\n", numLines - 1);
      }
    }
    result << "\n\n";
  }
  // instantiation of the design top module
  const auto input_names = m_design->get_top_module().get_input_names();
  const auto output_names = m_design->get_top_module().get_output_names();

  std::vector<std::string> port_assignments;
  port_assignments.reserve((input_names.size() * 2) + (output_names.size() * 2));

  for (const auto& element: ranges::views::concat(input_names, output_names)) {
    port_assignments.emplace_back(fmt::format(".{}({})", legalize_identifier(element), legalize_identifier(element)));
    if (ift) {
      port_assignments.emplace_back(fmt::format(".{}{}({}{})", legalize_identifier(element), ift_tag, legalize_identifier(element), ift_tag));
    }
  }

  result << fmt::format("{}  Ins({});\n\n", m_design->get_top_module().get_name(), boost::join(port_assignments, ", "));

  // verilog code for reading in text file containing the tags
  if (ift) {
    if (numLines > 0) {
      result << "initial begin \n";
      for (const auto& input_name: input_names) {
        result << fmt::format("{}{} = {}'b0;\n", legalize_identifier(input_name), ift_tag, tag_size);
      }
      result << fmt::format("\t$readmemb(`INPUT_FILE_NAME, read_simulation_data);\n");

      std::vector<std::string> read_ports;
      read_ports.reserve(input_names.size());
      for (const auto& input_name: input_names) {
        read_ports.emplace_back(fmt::format("{}{}", legalize_identifier(input_name), ift_tag));
      }

      result << fmt::format("\t\tt = read_simulation_data[0][{}:{}];\n", (tag_size * (read_ports.size() + 1) - 1) - (tag_size * read_ports.size()), ((tag_size * (read_ports.size() + 1) - 1) - (tag_size * read_ports.size())) - (tag_size - 1));
      result << "\t\t#t;\n";// time test to correspond to vcd file.  May have to change later

      result << fmt::format("\t\tfor(i = 1; i <= {}; i++) begin\n", numLines - 1);

      for (const auto& [index, port]: std::views::enumerate(read_ports)) {
        auto uindex = gsl::narrow<uint64_t>(index);
        auto port_start_bit = (tag_size * (read_ports.size() + 1)) - 1 - (tag_size * gsl::narrow<uint64_t>(uindex));
        auto port_end_bit = port_start_bit - (tag_size - 1);
        result << fmt::format("\t\t{} = read_simulation_data[i][{}:{}];\n", port, port_start_bit, port_end_bit);
      }
      result << fmt::format("\t\tt = read_simulation_data[i][{}:{}];\n", (tag_size * (read_ports.size() + 1) - 1) - (tag_size * read_ports.size()), ((tag_size * (read_ports.size() + 1) - 1) - (tag_size * read_ports.size())) - (tag_size - 1));
      // result << fmt::format("\t\t{{{}, t}} = read_simulation_data[i];\n", boost::join(read_ports, ", "));
      result << "\t\t#t;\n";// time test to correspond to vcd file.  May have to change later
      for (const auto& input_name: input_names) {
        result << fmt::format("{}{} = {}'b0;\n", legalize_identifier(input_name), ift_tag, tag_size);
      }
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

    result << fmt::format("assign tag_0 =  (^tag === 1'bx) ? 1 :  tag;\n");
    for (const auto& [index, port]: std::views::enumerate(tagged_ports)) {
      result << fmt::format("assign s{} = {} != {};\n", index, m_design->get_top_module().get_port(port.first.signal_name).m_name, m_design->get_top_module().get_port(port.second.signal_name).m_name);
      result << fmt::format("assign {}{} = s{} ? tag_{} : 0;\n", m_design->get_top_module().get_port(port.first.signal_name).m_name, ift_tag, index, index);
      result << fmt::format("assign tag_{} = s{} ? tag_{} << 1 : tag_{};\n", index + 1, index, index, index);
    }

    for (const auto& port: m_design->get_top_module().get_ports()) {
      if (!tagged_ports.contains(TagInjectTargetSignal(port.m_name)) && port.m_direction == ducode::Port::Direction::input) {
        result << fmt::format("assign {}{} = 0;\n", port.m_name, ift_tag);
      }
    }

    result << fmt::format("always @(posedge clk) begin\n");
    result << fmt::format("    tag = tag_{};\n", tagged_ports.size());
    result << fmt::format("end\n");
  }

  result << write_testbench_inputs();
  return result.str();
}

[[nodiscard]] std::string Testbench::write_testbench_internals_internal_module_tagging(const nlohmann::json& params) const {
  Expects(params.contains("tag_size"));
  Expects(params.contains("ift"));
  Expects(params.contains("tagged_module_name"));

  std::string tagged_module_name;
  if (params.contains("tagged_module_name")) {
    tagged_module_name = params["tagged_module_name"].get<std::string>();
  }

  auto tagged_module = std::ranges::find_if(m_design->get_modules(), [&](const auto& mod) { return mod.get_name() == "assignerCoreSec"; });
  assert(tagged_module != m_design->get_modules().end());

  const uint32_t tag_size = params["tag_size"].get<uint32_t>();
  const bool ift = params["ift"].get<bool>();

  std::stringstream result;

  result << "integer i;\n";
  result << "integer t;\n\n";

  uint32_t numLines = 0;
  if (ift && tagged_ports.empty()) {
    Expects(params.contains("timesteps_per_simulation_run"));
    numLines = params["timesteps_per_simulation_run"];

    // creating verilog variables for reading in the tags from external file
    if (numLines > 0) {
      const std::size_t read_simulation_data_lines = (tag_size * tagged_module->get_output_names().size()) + tag_size;
      result << fmt::format("reg [{}:0] ", ((read_simulation_data_lines - 1)));
      if (numLines <= 1) {
        result << "read_simulation_data [0:0];\n";
      } else {
        result << fmt::format("read_simulation_data [0:{}];\n", numLines - 1);
      }
    }
    result << "\n\n";
  }
  // instantiation of the design top module
  const auto input_names = m_design->get_top_module().get_input_names();
  const auto output_names_tagged_module = tagged_module->get_output_names();
  const auto output_names = m_design->get_top_module().get_output_names();

  std::vector<std::string> port_assignments;
  port_assignments.reserve((input_names.size() * 2) + (output_names.size() * 2));

  for (const auto& element: ranges::views::concat(input_names, output_names)) {
    port_assignments.emplace_back(fmt::format(".{}({})", legalize_identifier(element), legalize_identifier(element)));
    if (ift) {
      port_assignments.emplace_back(fmt::format(".{}{}({}{})", legalize_identifier(element), ift_tag, legalize_identifier(element), ift_tag));
    }
  }

  result << fmt::format("{}  Ins({});\n\n", m_design->get_top_module().get_name(), boost::join(port_assignments, ", "));

  // verilog code for reading in text file containing the tags
  if (ift) {
    if (numLines > 0) {
      result << "initial begin \n";
      for (const auto& output_name: output_names_tagged_module) {
        result << fmt::format("Ins.sm.ac.{}{} = {}'b0;\n", legalize_identifier(output_name), ift_tag, tag_size);
      }
      result << fmt::format("\t$readmemb(`INPUT_FILE_NAME, read_simulation_data);\n");

      std::vector<std::string> read_ports;
      read_ports.reserve(output_names_tagged_module.size());
      for (const auto& output_name: output_names_tagged_module) {
        read_ports.emplace_back(fmt::format("force Ins.sm.ac.{}{}", legalize_identifier(output_name), ift_tag));
      }

      result << fmt::format("\t\tt = read_simulation_data[0][{}:{}];\n", (tag_size * (read_ports.size() + 1) - 1) - (tag_size * read_ports.size()), ((tag_size * (read_ports.size() + 1) - 1) - (tag_size * read_ports.size())) - (tag_size - 1));
      result << "\t\t#t;\n";// time test to correspond to vcd file.  May have to change later

      result << fmt::format("\t\tfor(i = 1; i <= {}; i++) begin\n", numLines - 1);

      for (const auto& [index, port]: std::views::enumerate(read_ports)) {
        auto uindex = gsl::narrow<uint64_t>(index);
        auto port_start_bit = (tag_size * (read_ports.size() + 1)) - 1 - (tag_size * gsl::narrow<uint64_t>(uindex));
        auto port_end_bit = port_start_bit - (tag_size - 1);
        result << fmt::format("\t\t{} = read_simulation_data[i][{}:{}];\n", port, port_start_bit, port_end_bit);
      }
      result << fmt::format("\t\tt = read_simulation_data[i][{}:{}];\n", (tag_size * (read_ports.size() + 1) - 1) - (tag_size * read_ports.size()), ((tag_size * (read_ports.size() + 1) - 1) - (tag_size * read_ports.size())) - (tag_size - 1));
      // result << fmt::format("\t\t{{{}, t}} = read_simulation_data[i];\n", boost::join(read_ports, ", "));
      result << "\t\t#t;\n";// time test to correspond to vcd file.  May have to change later
      //update here if needed
      for (const auto& input_name: input_names) {
        result << fmt::format("{}{} = {}'b0;\n", legalize_identifier(input_name), ift_tag, tag_size);
      }
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

    result << fmt::format("assign tag_0 =  (^tag === 1'bx) ? 1 :  tag;\n");
    for (const auto& [index, port]: std::views::enumerate(tagged_ports)) {
      result << fmt::format("assign s{} = {} != {};\n", index, m_design->get_top_module().get_port(port.first.signal_name).m_name, m_design->get_top_module().get_port(port.second.signal_name).m_name);
      result << fmt::format("assign {}{} = s{} ? tag_{} : 0;\n", m_design->get_top_module().get_port(port.first.signal_name).m_name, ift_tag, index, index);
      result << fmt::format("assign tag_{} = s{} ? tag_{} << 1 : tag_{};\n", index + 1, index, index, index);
    }

    for (const auto& port: m_design->get_top_module().get_ports()) {
      if (!tagged_ports.contains(TagInjectTargetSignal(port.m_name)) && port.m_direction == ducode::Port::Direction::input) {
        result << fmt::format("assign {}{} = 0;\n", port.m_name, ift_tag);
      }
    }

    result << fmt::format("always @(posedge clk) begin\n");
    result << fmt::format("    tag = tag_{};\n", tagged_ports.size());
    result << fmt::format("end\n");
  }

  result << write_testbench_inputs();
  return result.str();
}

std::string Testbench::export_testbench(const nlohmann::json& params) const {
  Expects(params.contains("tag_size"));
  Expects(params.contains("ift"));

  const uint32_t tag_size = params["tag_size"].get<uint32_t>();
  const bool ift = params["ift"].get<bool>();

  std::optional<int> dumpvars_lvl;
  if (params.contains("dumpvars_lvl")) {
    dumpvars_lvl = params["dumpvars_lvl"].get<int>();
  }

  const auto& top_module = m_design->get_module(m_design->get_top_module_name());

  std::stringstream testbench;

  // header section

  testbench << fmt::format("module {}_tb;\n", top_module.get_name());

  for (const auto& port: top_module.get_ports()) {
    std::string width_string = fmt::format(" [{}:0] ", port.m_bits.size() - 1);
    std::string type_string = "wire";
    if (port.m_direction == Port::Direction::input) {
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
  //testbench << write_testbench_internals(params);

  if (params.contains("tagged_module_name")) {
    //auto tagged_module_name=params["tagged_module_name"].get<std::vector<std::string>>();
    // internals section
    testbench << write_testbench_internals_internal_module_tagging(params);
  } else {
    // internals section
    testbench << write_testbench_internals(params);
  }

  // footer section
  testbench << "initial\nbegin\n";
  testbench << "\t$dumpfile(`DUMP_FILE_NAME);\n";
  testbench << fmt::format("\t$dumpvars({});\n", dumpvars_lvl.value_or(0));
  testbench << "end\n\n\nendmodule";
  return testbench.str();
}

std::string Testbench::export_tags(const std::vector<SignalTime>& tags_times, nlohmann::json& params) const {
  Expects(params.contains("tag_size"));
  const uint32_t tag_size = params["tag_size"].get<uint32_t>();
  const ducode::Stepsize stepsize(params.value("stepsize", 1u));

  int number_of_injected_tags = 0;

  if (tag_size < tagged_ports.size()) {
    throw std::runtime_error("Tag size must be greater than tagged ports.");
  }

  std::stringstream oss;

  boost::dynamic_bitset<> tag_line_0(
      (m_design->get_module(m_design->get_top_module_name()).get_input_names().size() * tag_size) + tag_size,
      gsl::narrow<uint64_t>(tags_times[0]));

  oss << tag_line_0 << "\n";
  SignalTime prev_delta = 0;

  for (const auto& [idx, tag_time]: std::views::enumerate(tags_times)) {
    // Calculate current delta
    SignalTime delta = 0;
    if (idx < gsl::narrow<int64_t>(tags_times.size()) - 1) {
      // Not the last element - calculate delta normally
      delta = tags_times[static_cast<size_t>(idx) + 1] - tag_time;
      prev_delta = delta;// Store for use in the last iteration
    } else {
      // Last element - use the same delta as the previous iteration
      delta = prev_delta;
    }

    boost::dynamic_bitset<> tag_line(
        (m_design->get_module(m_design->get_top_module_name()).get_input_names().size() * tag_size) + tag_size,
        gsl::narrow<uint64_t>(std::max(gsl::narrow<uint32_t>(delta), stepsize.value())));// especially for random tagging, some clock cycles (timesteps) might be skipped, so the delta might be larger than the stepsize but must never be smaller


    for (const auto& [index, port]: std::views::enumerate(m_design->get_module(m_design->get_top_module_name()).get_input_names())) {
      if (m_tag_generator->is_tagged(m_design->get_module(m_design->get_top_module_name()).get_port(port), tag_time)) {
        auto bit_position = gsl::narrow<uint32_t>(tag_line.size() - tag_size) - ((index * tag_size) - index) + (idx * gsl::narrow<uint32_t>(m_design->get_module(m_design->get_top_module_name()).get_input_names().size()));
        tag_line[gsl::narrow<uint64_t>(bit_position)] = true;
        number_of_injected_tags++;
      }
    }
    oss << tag_line << "\n";
  }
  params["number_of_injected_tags"] = number_of_injected_tags;
  return oss.str();
}

std::string Testbench::export_tags_internal_module_tagging(const std::vector<SignalTime>& tags_times, nlohmann::json& params) const {
  Expects(params.contains("tag_size"));
  const uint32_t tag_size = params["tag_size"].get<uint32_t>();
  const ducode::Stepsize stepsize(params.value("stepsize", 1u));

  Expects(params.contains("tagged_module_name"));
  auto tagged_module_name = params["tagged_module_name"].get<std::string>();

  int number_of_injected_tags = 0;

  if (tag_size < tagged_ports.size()) {
    throw std::runtime_error("Tag size must be greater than tagged ports.");
  }

  std::stringstream oss;

  boost::dynamic_bitset<> tag_line_0(
      (m_design->get_module(tagged_module_name).get_output_names().size() * tag_size) + tag_size,
      gsl::narrow<uint64_t>(tags_times[0]));

  oss << tag_line_0 << "\n";
  SignalTime prev_delta = 0;

  for (const auto& [idx, tag_time]: std::views::enumerate(tags_times)) {
    // Calculate current delta
    SignalTime delta = 0;
    if (idx < gsl::narrow<int64_t>(tags_times.size()) - 1) {
      // Not the last element - calculate delta normally
      delta = tags_times[static_cast<size_t>(idx) + 1] - tag_time;
      prev_delta = delta;// Store for use in the last iteration
    } else {
      // Last element - use the same delta as the previous iteration
      delta = prev_delta;
    }

    boost::dynamic_bitset<> tag_line(
        (m_design->get_module(tagged_module_name).get_output_names().size() * tag_size) + tag_size,
        gsl::narrow<uint64_t>(std::max(gsl::narrow<uint32_t>(delta), stepsize.value())));// especially for random tagging, some clock cycles (timesteps) might be skipped, so the delta might be larger than the stepsize but must never be smaller


    for (const auto& [index, port]: std::views::enumerate(m_design->get_module(tagged_module_name).get_output_names())) {
      if (m_tag_generator->is_tagged(m_design->get_module(tagged_module_name).get_port(port), tag_time)) {
        auto bit_position = gsl::narrow<uint32_t>(tag_line.size() - tag_size) - ((index * tag_size) - index) + (idx * gsl::narrow<uint32_t>(m_design->get_module(m_design->get_top_module_name()).get_output_names().size()));
        tag_line[gsl::narrow<uint64_t>(bit_position)] = true;
        number_of_injected_tags++;
      }
    }
    oss << tag_line << "\n";
  }
  params["number_of_injected_tags"] = number_of_injected_tags;
  return oss.str();
}

[[nodiscard]] std::string TestbenchSMT::write_testbench_inputs() const {
  std::vector<SignalIdentifier> input_ids = get_design().get_input_signal_ids();
  std::stringstream result;

  result << "initial begin \n";

  // write the input signals
  // get_timestamps => function in SignalValueMap
  for (const auto& time: get_signal_map().get_timestamps()) {
    for (const SignalIdentifier& signal_id: input_ids) {
      if (signal_id.name == get_design().get_clk_signal_name()) {
        continue;
      }
      SignalInfo signal = get_signal_map().get_signal(signal_id);
      auto value = get_signal_map().get_signal_value(signal_id, static_cast<uint32_t>(time));
      auto bitValues = ducode::bit_to_vector(value);
      result << fmt::format("{} = {}'b", signal.reference, signal.size);
      if (bitValues.size() == 1 && bitValues[0] == SignalBit::BIT_X) {
        for (uint64_t i = 0; i < (static_cast<uint64_t>(signal.size) - bitValues.size()); i++) {
          result << "X";
        }
      } else {
        for (uint64_t i = 0; i < (static_cast<uint64_t>(signal.size) - bitValues.size()); i++) {
          result << "0";
        }
      }
      for (const auto& bit: bitValues) {
        result << SignalValue::signal_bit_to_char(bit);
      }
      result << ";\n";
    }
    if (time == 0) {
      result << fmt::format("#{};\n", constClockToggle * 2 + 1);//in time step 0, the inputs are immediately set and kept constant until passing the rising clock edge
    } else {
      // every unroll step in smt is represented by constClockToggle*2 steps in simulation; the inputs are set one quarter cycle after the rising edge of the clock
      result << fmt::format("#{};\n", constClockToggle * 2);
    }
  }
  result << "$finish;\n";
  result << "end\n\n";
  return result.str();
}

[[nodiscard]] std::string TestbenchSMT::write_clock() const {
  std::stringstream result;
  auto clk_signal_name = get_design().get_clk_signal_name();
  if (clk_signal_name != std::string("")) {
    result << "initial begin\n";
    result << "    " << clk_signal_name << " = 1;\n";
    result << "    forever #" << constClockToggle << " " << clk_signal_name << " = ~" << clk_signal_name << ";\n";
    result << "end\n\n";
  }
  return result.str();
}

[[nodiscard]] std::string TestbenchSMT::write_initial_state() const {
  std::stringstream result;
  const auto flipflop_ids = m_instance->get_all_flipflop_ids();

  if (!flipflop_ids.empty()) {
    result << "initial begin\n";
    result << "#0\n";
    //read out the flipflop values from the solver...
    for (const auto& flipflop_id: flipflop_ids) {
      const auto& signal = get_signal_map().get_signal(flipflop_id);
      const auto& value = get_signal_map().get_signal_value(flipflop_id, 0);
      auto bitValues = ducode::bit_to_vector(value);

      result << fmt::format("\tforce {}.{} = {}'b", boost::algorithm::join(flipflop_id.hierarchy, "."), legalize_identifier(flipflop_id.name), signal.size);
      if (bitValues.size() == 1 && bitValues[0] == SignalBit::BIT_X) {
        for (uint64_t i = 0; i < (static_cast<uint64_t>(signal.size) - bitValues.size()); i++) {
          result << "X";
        }
      } else {
        for (uint64_t i = 0; i < (static_cast<uint64_t>(signal.size) - bitValues.size()); i++) {
          result << "0";
        }
      }
      for (const auto& bit: bitValues) {
        result << SignalValue::signal_bit_to_char(bit);
      }
      result << ";\n";
    }
    result << "#1\n";
    for (const auto& flipflop_id: flipflop_ids) {
      result << fmt::format("\trelease {}.{};\n", boost::algorithm::join(flipflop_id.hierarchy, "."), legalize_identifier(flipflop_id.name));
    }
    result << "end\n\n";
  }
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
   * @param tagged_ports A map of tagged ports.
   * @param params Additional parameters for the testbench.
   * @return A string containing the generated verilog code for the testbench internals.
   */
[[nodiscard]] std::string TestbenchSMT::write_testbench_internals(const nlohmann::json& /*params*/) const {
  std::stringstream result;

  uint32_t numLines = 0;

  // instantiation of the design top module
  const auto input_names = get_design().get_top_module().get_input_names();
  const auto output_names = get_design().get_top_module().get_output_names();
  const auto flipflop_names = m_instance->get_all_flipflop_ids();

  std::vector<std::string> port_assignments;
  port_assignments.reserve((input_names.size() * 2) + (output_names.size() * 2));

  for (const auto& element: ranges::views::concat(input_names, output_names)) {
    port_assignments.emplace_back(fmt::format(".{}({})", legalize_identifier(element), legalize_identifier(element)));
  }

  result << fmt::format("{}  {}({});\n\n", get_design().get_top_module().get_name(), get_design().get_top_module().get_name(), boost::join(port_assignments, ", "));

  result << write_clock();

  result << write_initial_state();

  result << write_testbench_inputs();
  return result.str();
}

std::string TestbenchSMT::export_testbench(const nlohmann::json& params) const {
  std::optional<int> dumpvars_lvl;
  if (params.contains("dumpvars_lvl")) {
    dumpvars_lvl = params["dumpvars_lvl"].get<int>();
  }

  const auto& top_module = get_design().get_module(get_design().get_top_module_name());

  std::stringstream testbench;

  // header section

  testbench << fmt::format("module {}_tb;\n", top_module.get_name());

  for (const auto& port: top_module.get_ports()) {
    std::string width_string = fmt::format(" [{}:0] ", port.m_bits.size() - 1);
    std::string type_string = "wire";
    if (port.m_direction == Port::Direction::input) {
      type_string = "reg";
    }
    testbench << fmt::format("{} {}{};\n", type_string, width_string, legalize_identifier(port.m_name));
  }

  // internals section
  testbench << write_testbench_internals(params);

  // footer section
  testbench << "initial\nbegin\n";
  testbench << "\t$dumpfile(`DUMP_FILE_NAME);\n";
  testbench << fmt::format("\t$dumpvars({});\n", dumpvars_lvl.value_or(0));
  testbench << "end\n\n\nendmodule";
  return testbench.str();
}

}// namespace ducode
