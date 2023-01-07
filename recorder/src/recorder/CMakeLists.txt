# cmake file for src/recorder

# external dependencies
find_package(spdlog REQUIRED)
set(DEPENDENCIES spdlog::spdlog)

# source files
file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp *.ixx)
set(SOURCES ${SOURCES})

# build target recorder (executable)
add_executable(recorder ${SOURCES})
target_link_libraries(recorder PRIVATE ${DEPENDENCIES})

set_default_warnings(recorder)

enable_sanitizers(recorder
  "${ENABLE_SANITIZER_ADDRESS}"
  "${ENABLE_SANITIZER_LEAK}"
  "${ENABLE_SANITIZER_UNDEFINED_BEHAVIOR}"
  "${ENABLE_SANITIZER_THREAD}"
  "${ENABLE_SANITIZER_MEMORY}")

if(ENABLE_COVERAGE)
  enable_coverage(recorder)
endif()

