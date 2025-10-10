/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/design.hpp>
#include <ducode/signals_data_manager.hpp>
#include <ducode/tag_generator.hpp>
#include <ducode/utility/concepts.hpp>
#include <ducode/utility/types.hpp>
#include <ducode/utility/verilog_parser.hpp>

#include "ducode/instantiation_graph.hpp"
#include <fmt/core.h>
#include <nlohmann/json_fwd.hpp>
#include <z3++.h>

#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

namespace ducode {

/**
   * @brief The Testbench class represents a testbench for a design.
   *
   * This class provides functionality to write Verilog testbenches and generate test vectors
   * based on VCD (Value Change Dump) files.
   */
class Testbench {
  Design const *m_design;
  std::shared_ptr<SignalsDataManager> m_signal_tag_map;
  //Not used: Design m_testbench;
  std::unordered_map<TagInjectTargetSignal, TagTriggerConditionSignal> tagged_ports;// only used in the experiments for the approx_ift paper
  std::unique_ptr<TagGenerator> m_tag_generator;

public:
  virtual ~Testbench() = default;
  Testbench(const Testbench &) = delete;
  Testbench(Testbench &&) = delete;
  Testbench &operator=(const Testbench &) = delete;
  Testbench &operator=(Testbench &&) = delete;
  /**
     * @brief Constructs a Testbench object.
     *
     * This constructor initializes a Testbench object with the given design and VCD data.
     *
     * @param design The design object associated with the testbench.
     * @param signal_tag_map A shared pointer to SignalsDataManager object containing the VCD or SMT backend.
     */
  Testbench(const Design &design, std::shared_ptr<SignalsDataManager> signal_tag_map) : m_design(&design),
                                                                                        m_signal_tag_map(std::move(signal_tag_map)) {
  }

  /* Not used
  template<filesystem_like T>
  Testbench(const T &original_testbench, std::shared_ptr<SignalsDataManager> signal_tag_map) : m_design(nullptr),
                                                                                               m_signal_tag_map(std::move(signal_tag_map)) {
    auto json = parse_verilog(original_testbench);
    m_testbench = Design::parse_json(json);
  }*/

  void set_tag_generator(std::unique_ptr<TagGenerator> tag_generator) {
    m_tag_generator = std::move(tag_generator);
  }

  void add_tags(const std::unordered_map<TagInjectTargetSignal, TagTriggerConditionSignal> &p_tagged_ports) {
    tagged_ports = p_tagged_ports;
  }

  uint32_t get_number_of_injected_tags() const {
    if (m_tag_generator) {
      return m_tag_generator->get_number_of_injected_tags();
    }
    return 0;
  }

  [[nodiscard]] const Design &get_design() const {
    Expects(m_design != nullptr);
    return *m_design;
  }

  [[nodiscard]] const SignalsDataManager &get_signal_map() const {
    Expects(m_signal_tag_map != nullptr);
    return *m_signal_tag_map;
  }

  /**
     * Writes the Verilog code for the given testbench and parameters.
     *
     * @param testbench The testbench to write Verilog code for.
     * @param params The parameters for the Verilog code generation.
     */
  void write_verilog(const filesystem_like auto &testbench, const nlohmann::json &params) const {
    std::ofstream testbench_file(testbench.string());
    if (!testbench_file.is_open()) {
      throw std::runtime_error(fmt::format("Could not open file: {}", testbench.string()));
    }
    testbench_file << export_testbench(params);
    testbench_file.close();
  }

  void write_tags(const std::vector<ExportedTagsFile> &tags_intervals,
                  nlohmann::json &params) const {
    for (const auto &[tags_times, tags_file]: tags_intervals) {
      std::ofstream testbench_file(tags_file.string());
      if (!testbench_file.is_open()) {
        throw std::runtime_error(fmt::format("Could not open file: {}", tags_file.string()));
      }
      if (params.contains("tagged_module_name")) {
        testbench_file << export_tags_internal_module_tagging(tags_times, params);
      } else {
        testbench_file << export_tags(tags_times, params);
      }
      //testbench_file << export_tags(tags_times, params);
      testbench_file.close();
    }
  }

private:
  [[nodiscard]] virtual std::string write_testbench_internals(const nlohmann::json &params) const;
  [[nodiscard]] std::string write_testbench_internals_internal_module_tagging(const nlohmann::json &params) const;

  [[nodiscard]] virtual std::string write_testbench_inputs() const;

protected:
  /**
     * @brief Generates a testbench module for a given design.
     *
     * This function generates a testbench module based on the provided design. The testbench module includes the necessary
     * declarations, internal logic, and initialization code for the testbench. The generated testbench module is returned
     * as a string.
     *
     * @param params Additional parameters for the testbench.
     * @return The generated testbench module as a string.
     *
     * @throws std::runtime_error if the design does not contain a top module or if the testbench contains more than one
     * instantiated design.
     */
  [[nodiscard]] virtual std::string export_testbench(const nlohmann::json &params) const;

  [[nodiscard]] std::string export_tags(const std::vector<SignalTime> &tags_times, nlohmann::json &params) const;
  [[nodiscard]] std::string export_tags_internal_module_tagging(const std::vector<SignalTime> &tags_times, nlohmann::json &params) const;
};

class TestbenchSMT : public Testbench {
  const uint32_t constClockToggle = 2;
  const DesignInstance *m_instance = nullptr;

public:
  ~TestbenchSMT() override = default;
  TestbenchSMT(const TestbenchSMT &) = delete;
  TestbenchSMT(TestbenchSMT &&) = delete;
  TestbenchSMT &operator=(const TestbenchSMT &) = delete;
  TestbenchSMT &operator=(TestbenchSMT &&) = delete;
  /* Not used
  TestbenchSMT(const Design &design, std::shared_ptr<SMTSignalsDataManager> signal_tag_map) : Testbench(design, std::move(signal_tag_map)) {
  }
  */
  TestbenchSMT(const Design &design, std::shared_ptr<SMTSignalsDataManager> signal_tag_map, DesignInstance &instance) : Testbench(design, std::move(signal_tag_map)), m_instance(&instance) {
  }

private:
  [[nodiscard]] std::string write_testbench_internals(const nlohmann::json &params) const override;
  [[nodiscard]] std::string write_testbench_inputs() const override;
  [[nodiscard]] std::string write_initial_state() const;
  [[nodiscard]] std::string write_clock() const;

protected:
  /**
     * @brief Generates a testbench module for a given design.
     *
     * This function generates a testbench module based on the provided design. The testbench module includes the necessary
     * declarations, internal logic, and initialization code for the testbench. The generated testbench module is returned
     * as a string.
     *
     * @param params Additional parameters for the testbench.
     * @return The generated testbench module as a string.
     *
     * @throws std::runtime_error if the design does not contain a top module or if the testbench contains more than one
     * instantiated design.
     */
  [[nodiscard]] std::string export_testbench(const nlohmann::json &params) const override;
};

}// namespace ducode
