//
// Created by lutz on 13.02.23.
//
#pragma once

#include <common_definitions.hpp>
#include <ducode/utility/concepts.hpp>
#include <ducode/utility/replace.hpp>

#include <boost/filesystem.hpp>
#include <gsl/assert>
#include <spdlog/spdlog.h>
#include <vcd-parser/VCDFileParser.hpp>

#include <filesystem>
#include <iostream>
#include <string>

namespace ducode {

/// function for simulation of verilog designs with IVerilog
/// takes the paths to the design file and the testbench file as parameters
/// returns the VCD data which results from the simulation
namespace detail {

inline std::shared_ptr<VCDFile> simulate_design(const std::string_view search_path, const std::string_view design_file, const std::string_view testbench_file, const std::string_view input_file = "") {
  std::string iverilog = fmt::format("iverilog -I{}", search_path);
  std::string vvp = "vvp";

#if (defined(_WIN32) || defined(_WIN64))
  iverilog = fmt::format("set \"PATH={0}/bin;{0}/lib;%PATH%\" && {1}", OSSCAD_DIR, iverilog);
  vvp = fmt::format("set \"PATH={0}/bin;{0}/lib;%PATH%\" && {1}", OSSCAD_DIR, vvp);
#endif

  auto temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  auto created = boost::filesystem::create_directories(temp_dir);
  Ensures(created);

  auto vvpFile = temp_dir / "output.vvp";
  auto vcdFile = temp_dir / "output.vcd";
  auto vcd_file_string = vcdFile.string();
  std::string input_file_string{input_file};

#if (defined(_WIN32) || defined(_WIN64))
  replace_all(vcd_file_string, "\\", "\\\\");
  replace_all(vcd_file_string, "/", "//");
  replace_all(input_file_string, "\\", "\\\\");
  replace_all(input_file_string, "/", "//");

#endif


  std::string iverilog_cmd;
  if (input_file.empty()) {
    iverilog_cmd = fmt::format(R"({} -o {} -DDUMP_FILE_NAME=\"{}\" {} {})", iverilog, vvpFile.string(), vcd_file_string, design_file, testbench_file);
  } else {
    iverilog_cmd = fmt::format(R"({} -o {} -DINPUT_FILE_NAME=\"{}\" -DDUMP_FILE_NAME=\"{}\" {} {})", iverilog, vvpFile.string(), input_file_string, vcd_file_string, design_file, testbench_file);
  }
  spdlog::info("Executing iverilog");
  spdlog::info("Executing command: {}", iverilog_cmd);

  if (!std::filesystem::exists(design_file) || !std::filesystem::exists(testbench_file)) {
    throw std::runtime_error("design or testbench files are missing!!!");
  }

  int system_result = std::system(iverilog_cmd.c_str());//NOLINT(cert-env33-c,concurrency-mt-unsafe,clang-analyzer-deadcode.DeadStores)
  if (!std::filesystem::exists(vvpFile.string())) {
    throw std::runtime_error("vvp file was not created!");
  }

  ///vvp command resulting in the vcd file for the simulation.
  std::string vvp_cmd = fmt::format("{} {}", vvp, vvpFile.string());
  spdlog::info("Executing VVP simulation file");
  spdlog::info("Executing command: {}", vvp_cmd);
  system_result = std::system(vvp_cmd.c_str());//NOLINT(cert-env33-c,concurrency-mt-unsafe,clang-analyzer-deadcode.DeadStores)

  if (!std::filesystem::exists(vcdFile.string())) {
    throw std::runtime_error("vcd file was not created!");
  }
  spdlog::info("Simulation complete!");
  VCDFileParser vcd_obj;
  return vcd_obj.parse_file(vcdFile.string());
}

}// namespace detail

template<filesystem_like T>
inline std::shared_ptr<VCDFile> simulate_design(const T& search_path, const T& design_file, const T& testbench_file, const T& input_file) {
  return detail::simulate_design(search_path.string(), design_file.string(), testbench_file.string(), input_file.string());
}

template<filesystem_like T>
inline std::shared_ptr<VCDFile> simulate_design(const T& design_file, const T& testbench_file, const T& input_file) {
  return detail::simulate_design(design_file.parent_path().string(), design_file.string(), testbench_file.string(), input_file.string());
}

template<filesystem_like T>
inline std::shared_ptr<VCDFile> simulate_design(const T& design_file, const T& testbench_file) {
  return detail::simulate_design(design_file.parent_path().string(), design_file.string(), testbench_file.string());
}

}// namespace ducode