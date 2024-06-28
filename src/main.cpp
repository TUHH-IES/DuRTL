/* SPDX-License-Identifier: Apache-2.0 */

#include <ducode/ift.hpp>

#include <CLI/CLI.hpp>
#include <boost/filesystem/path.hpp>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <cstdint>
#include <memory>
#include <string>

int main(int argc, char* argv[]) {
  CLI::App app{"Basic IFT execution"};
  std::string json_filename;
  std::string design_filename;
  std::string testbench_filename;
  std::string vcd_filename;
  uint32_t stepsize = 1;
  app.add_option("-j,--json", json_filename, "The json file exported from Yosys")->required()->check(CLI::ExistingFile);
  app.add_option("-d,--design", design_filename, "The design")->required()->check(CLI::ExistingFile);
  app.add_option("-t,--testbench", testbench_filename, "The testbench")->required()->check(CLI::ExistingFile);
  app.add_option("-v,--vcdfile", vcd_filename, "The vcdfile")->required()->check(CLI::ExistingFile);
  app.add_option("-s,--stepsize", stepsize, "The stepsize for the tag injection")->required()->check(CLI::PositiveNumber);
  CLI11_PARSE(app, argc, argv);

  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  console_sink->set_level(spdlog::level::info);
  console_sink->set_pattern("[%l] %^%s:%# %! %v");

  auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("main.log", true);
  file_sink->set_level(spdlog::level::trace);
  file_sink->set_pattern("[%l] %^%s:%# %! %v");
  const spdlog::sinks_init_list sink_list = {file_sink, console_sink};

  spdlog::logger logger("duLog", sink_list.begin(), sink_list.end());
  logger.set_level(spdlog::level::trace);
  spdlog::set_default_logger(std::make_shared<spdlog::logger>(logger));

  spdlog::debug("Configured LOGGER");

  auto json_file = boost::filesystem::path(json_filename);
  auto vcd_file = boost::filesystem::path(vcd_filename);
  ducode::ift(json_file, vcd_file, stepsize);

  spdlog::shutdown();
  return 0;
}
