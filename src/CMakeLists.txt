add_library(sources INTERFACE)
target_include_directories(sources INTERFACE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(sources INTERFACE
        configuration_files fmt::fmt Microsoft.GSL::GSL spdlog::spdlog Boost::filesystem
        nlohmann_json::nlohmann_json range-v3::range-v3 unordered_dense::unordered_dense
        vcd-parser::vcd-parser)
enable_sanitizers(sources)
set_project_warnings(sources)
if (NOT ENABLE_COVERAGE)
    target_precompile_headers(sources INTERFACE
            ${CMAKE_SOURCE_DIR}/include/ducode/module.hpp
            ${CMAKE_SOURCE_DIR}/include/ducode/design.hpp
            ${CMAKE_SOURCE_DIR}/include/ducode/cell.hpp
    )
    set_target_properties(sources PROPERTIES UNITY_BUILD ON)
endif ()

add_executable(main main.cpp)
target_link_libraries(main PRIVATE sources configuration_files CLI11::CLI11)

add_executable(exp_analysis experiments_analysis.cpp)
target_link_libraries(exp_analysis PRIVATE sources configuration_files Catch2::Catch2 vcd-parser::vcd-parser)

add_executable(exp_apprx experiments_apprx.cpp)
target_link_libraries(exp_apprx PRIVATE sources configuration_files Catch2::Catch2 vcd-parser::vcd-parser)
