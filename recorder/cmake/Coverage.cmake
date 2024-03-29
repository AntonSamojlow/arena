include_guard()

# Enable coverage reporting for gcc/clang
function(enable_coverage target_name)
  message(TRACE "[${target_name}] - function 'enable_coverage'")

  if(NOT ENABLE_GCOV AND NOT ENABLE_LLVMPROFILE)
    message(STATUS "[${target_name}] no coverage enabled")

  elseif(MSVC)
    message(SEND_ERROR "script does not support code coverage on windows")

  elseif(ENABLE_GCOV)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
      message(SEND_ERROR "gcov based coverage not supported for release builds")  # gcov requires -O0
    endif()
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
      message(STATUS "[${target_name}] gcov based coverage enabled")
      # target_compile_options(${target_name} PUBLIC --coverage -fprofile-abs-path -O0 -g)  <- only gcc
      target_compile_options(${target_name} PUBLIC --coverage -O0 -g)
      target_link_libraries(${target_name} PUBLIC --coverage)
    else()
      message(SEND_ERROR "gcov based coverage not available for compiler '${CMAKE_CXX_COMPILER_ID}'")
    endif()

  elseif(ENABLE_LLVMPROFILE)
    if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
        message(STATUS "[${target_name}] llvm profile based coverage enabled")
        target_compile_options(${target_name} PUBLIC -fprofile-instr-generate -fcoverage-mapping)
        target_link_libraries(${target_name} PUBLIC -fprofile-instr-generate)
      else()
        message(SEND_ERROR "llvm profile based coverage but not available for compiler '${CMAKE_CXX_COMPILER_ID}'")
    endif()
  endif()

endfunction()


