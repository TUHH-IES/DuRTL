add_library(sources STATIC
        ducode/design.cpp
        ducode/instantiation_graph.cpp
        ducode/module.cpp
        ducode/testbench.cpp
        ducode/utility/VCD_utility.cpp
        ducode/utility/verilog_parser.cpp
        ducode/signals_data_manager.cpp
)
target_include_directories(sources PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(sources PUBLIC
        configuration_files fmt::fmt Microsoft.GSL::GSL spdlog::spdlog Boost::filesystem
        nlohmann_json::nlohmann_json range-v3::range-v3 unordered_dense::unordered_dense yosys-verilog::yosys-verilog
        verible::verilog-token verible::verilog-tree-json verible::json-diagnostics verible::file-util verible::parser-verifier
        vcd-parser::vcd-parser z3::libz3)
enable_sanitizers(sources)
set_project_warnings(sources)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE sources CLI11::CLI11)

add_executable(exp_analysis experiments_analysis.cpp)
target_link_libraries(exp_analysis PRIVATE sources Catch2::Catch2)

add_executable(exp_apprx experiments_apprx.cpp)
target_link_libraries(exp_apprx PRIVATE sources Catch2::Catch2)

add_executable(exp_ifa experiments_ifa_ift.cpp)
target_link_libraries(exp_ifa PRIVATE sources Catch2::Catch2 CLI11::CLI11)