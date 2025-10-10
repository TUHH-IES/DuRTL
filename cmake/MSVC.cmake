if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    add_definitions(-DWORD_BIT=64)
  else()
    add_definitions(-DWORD_BIT=32)
  endif()

  set(OSSCAD_DIR ${CMAKE_SOURCE_DIR}/thirdparty/oss-cad-suite/)

  add_compile_options(/bigobj)
endif ()