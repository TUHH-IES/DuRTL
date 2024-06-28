cmake_minimum_required(VERSION 3.8)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/vcpkg/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")

project(ducode CXX)
set(CMAKE_CXX_STANDARD 20)

include(cmake/StandardProjectSettings.cmake)
include(cmake/PreventInSourceBuilds.cmake)
include(cmake/CompilerWarnings.cmake)
include(cmake/Sanitizers.cmake)
include(cmake/MSVC.cmake)

#Compiler flag configuration checks
include(CheckCXXCompilerFlag)

include(CTest)
include(cmake/Catch.cmake)
enable_testing()

find_package(fmt CONFIG REQUIRED)
find_package(Microsoft.GSL CONFIG REQUIRED)
find_package(CLI11 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(Catch2 CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem)
find_package(vcd-parser CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(range-v3 CONFIG REQUIRED)
find_package(unordered_dense CONFIG REQUIRED)

add_subdirectory(config)
add_subdirectory(src)
add_subdirectory(tests)