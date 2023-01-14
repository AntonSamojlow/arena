include_guard()

# Enable coverage reporting for gcc/clang
function(enable_coverage target_name)
  if(MSVC)
    message(SEND_ERROR "gcov based coverage not supported on windows")
  endif()
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # gcov requires setting the -O0 flag!
    message(SEND_ERROR "gcov based coverage not supported for release builds")
  endif()

  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    message(STATUS "coverage enabled")
    target_compile_options(${target_name} PRIVATE --coverage -O0 -g)
    target_link_libraries(${target_name} PRIVATE --coverage)
  else()
    message(SEND_ERROR "coverage enabled but compiler is '${CMAKE_CXX_COMPILER_ID}'")
  endif()
endfunction()