# cmake file for src/sag

# external dependencies
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
set(DEPENDENCIES spdlog::spdlog fmt::fmt)

# source files
file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp *.ixx)
set(SOURCES ${SOURCES})

# build target sag (library)
add_library(sag ${SOURCES})
target_link_libraries(sag PRIVATE spdlog::spdlog)
set_target_properties(sag PROPERTIES LINKER_LANGUAGE CXX)

set_default_warnings(sag)

enable_sanitizers(sag
  "${ENABLE_SANITIZER_ADDRESS}"
  "${ENABLE_SANITIZER_LEAK}"
  "${ENABLE_SANITIZER_UNDEFINED_BEHAVIOR}"
  "${ENABLE_SANITIZER_THREAD}"
  "${ENABLE_SANITIZER_MEMORY}")

if(ENABLE_COVERAGE)
  enable_coverage(sag)
endif()