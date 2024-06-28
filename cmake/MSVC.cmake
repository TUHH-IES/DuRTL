if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    add_definitions(-DWORD_BIT=64)
  else()
    add_definitions(-DWORD_BIT=32)
  endif()

  if (NOT EXISTS ${CMAKE_SOURCE_DIR}/thirdparty/win_flex_bison)
    message(FATAL_ERROR "WinFlexBison not installed properly. Read the README.md file for instructions.")
  endif ()

  if (NOT EXISTS ${CMAKE_SOURCE_DIR}/thirdparty/oss-cad-suite)
    message(FATAL_ERROR "OSS CAD Suite not installed properly. Read the README.md file for instructions.")
  endif ()

  LIST(APPEND CMAKE_PROGRAM_PATH "${CMAKE_SOURCE_DIR}/thirdparty/win_flex_bison")
  include_directories(${CMAKE_SOURCE_DIR}/thirdparty/win_flex_bison)

  set(OSSCAD_DIR ${CMAKE_SOURCE_DIR}/thirdparty/oss-cad-suite/)

  add_compile_options(/bigobj)
endif ()