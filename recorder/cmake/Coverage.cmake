include_guard()

# Enable coverage reporting for gcc/clang
function(enable_coverage target_name)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    message(STATUS "coverage enabled")
    target_compile_options(${target_name} INTERFACE --coverage -O0 -g)
    target_link_libraries(${target_name} INTERFACE --coverage)
  else()
    message(SEND_ERROR "coverage enabled but compiler is '${CMAKE_CXX_COMPILER_ID}'")
  endif()
endfunction()