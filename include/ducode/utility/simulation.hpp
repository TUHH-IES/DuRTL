/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <common_definitions.hpp>
#include <ducode/module.hpp>
#include <ducode/utility/iverilog_wrapper.hpp>
#include <ducode/utility/types.hpp>

#include <boost/filesystem.hpp>
#include <spdlog/spdlog.h>
#include <sys/types.h>
#include <vcd-parser/VCDComparisons.hpp>

#include <algorithm>
#include <string>

namespace ducode {
/**
           * @brief Calculates the step size of a clock signal in a VCD file.
           *
           * This function takes a VCD data object and the name of a clock signal as input.
           * It searches for the specified clock signal in the VCD data and retrieves its values.
           * The function then returns the step size of the clock signal, which is the time difference
           * between the third and second value in the signal's value history.
           *
           * @param vcd_data A reference to the VCD data object.
           * @param clk_name The name of the clock signal.
           * @return The step size of the clock signal.
           */

inline VCDTime get_stepsize(std::shared_ptr<VCDFile> &vcd_data, const std::string &clk_name,
                            const std::string &tb_name) {
  const VCDScope tb_scope = vcd_data->get_scope(tb_name);

  std::deque<VCDTimedValue> clk_values;

  for (const VCDSignal *signal: tb_scope.signals) {
    if (signal->reference == clk_name) {
      clk_values = vcd_data->get_signal_values(signal->hash);
    }
    if (!clk_values.empty()) {
      return clk_values[2].time;
    }
  }
  // if no signal 'clk_name' is found return a stepsize of 1 (for combinatorial designs) and print a warning in case 'clk_name' was wrong
  spdlog::warn("Signal {} NOT FOUND!", clk_name);
  return 1;
}

inline VCDTime get_stepsize(const std::shared_ptr<VCDFile> &vcd_data, const std::string &clk_name) {
  // scope 0 of the vcd file is a "root" scope
  std::vector<VCDScope *> child_scopes_tmp = vcd_data->get_scopes().at(0).children;
  // scope 0 of the children of the root scope should always be the testbench scope in the VCDfile
  VCDScope *child_scope = [&]() {
    for (auto *scope: child_scopes_tmp) {
      if (scope->name.find("_tb") != std::string::npos || scope->name.find("TB") != std::string::npos) {
        return scope;
      }
    }
    return static_cast<VCDScope *>(nullptr);
  }();
  assert(child_scope != nullptr);

  std::deque<VCDTimedValue> clk_values;

  // scope at position zero is the testbench scope of the VCDfile
  for (const VCDSignal *signal: child_scope->signals) {
    if (signal->reference == clk_name) {
      clk_values = vcd_data->get_signal_values(signal->hash);
      break;
    }
  }
  if (!clk_values.empty()) {
    // clk_values[0] is 0; stepsize should represent 1 full clockcycle so return the third time value of the clock signal
    return clk_values[2].time;
  }

  // if no signal 'clk_name' is found return a stepsize of 1 (for combinatorial designs) and print a warning in case 'clk_name' was wrong
  spdlog::warn("Signal {} NOT FOUND!", clk_name);
  return 1;
}

class Stepsize {
  uint32_t m_stepsize;

public:
  template<typename T>
    requires std::is_unsigned_v<T>
  explicit Stepsize(T stepsize) : m_stepsize(stepsize) {}

  explicit Stepsize(const std::shared_ptr<VCDFile> &vcd_data, const std::string &clk_name) {
    // scope 0 of the vcd file is a "root" scope
    std::vector<VCDScope *> child_scopes_tmp = vcd_data->get_scopes().at(0).children;
    // scope 0 of the children of the root scope should always be the testbench scope in the VCDfile
    assert(
        child_scopes_tmp.at(0)->name.find("_tb") != std::string::npos || child_scopes_tmp.at(0)->name.find("TB") !=
                                                                             std::
                                                                                 string::npos);

    std::deque<VCDTimedValue> clk_values;

    // scope at position zero is the testbench scope of the VCDfile
    for (const VCDScope *scope: vcd_data->get_scopes().at(0).children) {
      for (const VCDSignal *signal: scope->signals) {
        if (signal->reference == clk_name) {
          clk_values = vcd_data->get_signal_values(signal->hash);
          break;
        }
      }
      if (!clk_values.empty()) {
        // clk_values[0] is 0; stepsize should represent 1 full clockcycle so return the third time value of the clock signal
        m_stepsize = gsl::narrow<uint32_t>(clk_values[2].time);
      }
    }
    // if no signal 'clk_name' is found return a stepsize of 1 (for combinatorial designs) and print a warning in case 'clk_name' was wrong
    spdlog::warn("Signal {} NOT FOUND!", clk_name);
    m_stepsize = 1;
  }

  [[nodiscard]] uint32_t value() const { return m_stepsize; }
};


inline uint32_t get_timesteps_per_simulation_run(const Module &top_module, uint32_t tag_size) {
  Expects(tag_size > 0);
  auto timesteps_per_simulation_run = tag_size / top_module.get_input_names().size();
  timesteps_per_simulation_run = std::max<size_t>(timesteps_per_simulation_run, 1);
  return gsl::narrow<uint32_t>(timesteps_per_simulation_run);
}

inline uint32_t get_timesteps_per_simulation_run_internal_module_tagging(const Module &top_module, uint32_t tag_size) {
  Expects(tag_size > 0);
  auto timesteps_per_simulation_run = tag_size / top_module.get_output_names().size();
  timesteps_per_simulation_run = std::max<size_t>(timesteps_per_simulation_run, 1);
  return gsl::narrow<uint32_t>(timesteps_per_simulation_run);
}

inline uint32_t get_number_of_simulation_runs(const uint32_t stepsize, const uint32_t timesteps_per_simulation_run,
                                              const std::shared_ptr<VCDFile> &vcd_data) {
  Expects(stepsize > 0);
  Expects(timesteps_per_simulation_run > 0);
  Expects(vcd_data != nullptr);

  double number_simulation_runs = (gsl::narrow<double>(vcd_data->get_timestamps().back())) / gsl::narrow<double>(stepsize) /
                                  gsl::narrow<double>(timesteps_per_simulation_run);
  number_simulation_runs = std::ceil(number_simulation_runs);// Round up to ensure complete coverage
  auto needed_runs = gsl::narrow<uint32_t>(number_simulation_runs);
  assert(needed_runs * gsl::narrow<uint32_t>(stepsize) * gsl::narrow<uint32_t>(timesteps_per_simulation_run) >= gsl::narrow<uint32_t>(vcd_data->get_timestamps().back()));
  return needed_runs;
}

inline uint32_t get_number_of_simulation_runs(const Stepsize stepsize, const uint32_t timesteps_per_simulation_run,
                                              const std::shared_ptr<VCDFile> &vcd_data) {
  return get_number_of_simulation_runs(stepsize.value(), timesteps_per_simulation_run, vcd_data);
}

inline std::vector<boost::filesystem::path> get_exported_tags_files(uint32_t number_simulation_runs) {
  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  assert(created);
  std::vector<boost::filesystem::path> exported_tags_files;
  for (uint32_t i = 0; i < number_simulation_runs; ++i) {
    boost::filesystem::path tag_file = temp_dir / ("vcd_input_" + std::to_string(i) + ".txt");
    exported_tags_files.push_back(tag_file);
  }
  return exported_tags_files;
}

inline std::vector<VCDTime> get_tags_times(std::shared_ptr<VCDFile> &vcd_data, uint32_t stepsize) {
  Expects(vcd_data != nullptr);
  Expects(stepsize > 0);

  std::vector<VCDTime> tags_times;
  for (auto time = 0u; time <= gsl::narrow<std::size_t>(vcd_data->get_timestamps().back()); time += stepsize) {
    tags_times.emplace_back(time);
  }
  return tags_times;
}

inline std::vector<ExportedTagsFile> get_exported_tags_vec(
    const std::vector<boost::filesystem::path> &exported_tags_files, const uint32_t number_simulation_runs,
    const std::vector<VCDTime> &tags_times,
    const uint32_t timesteps_per_simulation_run) {
  std::vector<ExportedTagsFile> exported_tags_vec;

  for (uint32_t i = 0; i < number_simulation_runs; ++i) {
    // For the ith batch of timestamps
    uint32_t items_per_batch = timesteps_per_simulation_run - 1;
    uint32_t start_idx = i * items_per_batch;
    uint32_t end_idx = std::min(((i + 1) * items_per_batch), gsl::narrow<uint32_t>(tags_times.size()));

    std::vector<VCDTime> batch(tags_times.begin() + start_idx,
                               tags_times.begin() + end_idx);

    exported_tags_vec.emplace_back(batch, exported_tags_files[i]);
  }
  return exported_tags_vec;
}
}// namespace ducode
