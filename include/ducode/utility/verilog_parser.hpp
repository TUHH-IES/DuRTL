/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <ducode/utility/concepts.hpp>

#include <spdlog/spdlog.h>

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

namespace ducode {

std::string parse_verilog(const std::string& verilog_file, bool debug);
std::string parse_verilog(const std::vector<std::string>& verilog_files, bool debug);

template<filesystem_like T>
std::string parse_verilog(const T& verilog_file, bool debug = false) {
  spdlog::info("Parsing verilog file: {}", verilog_file.string());
  return parse_verilog(verilog_file.string(), debug);
}

template<filesystem_like T>
std::string parse_verilog(const std::vector<T>& verilog_files, bool debug = false) {
  std::vector<std::string> filenames;
  std::ranges::transform(verilog_files, std::back_inserter(filenames), [](const T& verilog_file) { return verilog_file.string(); });
  spdlog::info("Parsing verilog files: {}", verilog_files);
  return parse_verilog(filenames, debug);
}

std::vector<std::string> get_instances_names(std::string_view filename, std::string_view module_name);

}// namespace ducode