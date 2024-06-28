set(TESTFILES_DIR ${CMAKE_SOURCE_DIR}/tests/testfiles/)
configure_file(common_definitions.hpp.in common_definitions.hpp @ONLY)
add_library(configuration_files INTERFACE)
target_include_directories(configuration_files INTERFACE ${CMAKE_BINARY_DIR}/config)