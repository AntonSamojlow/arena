# cmake file for src/graph

# external dependencies
find_package(spdlog REQUIRED)
set(DEPENDENCIES spdlog::spdlog mylibrary)

# source files
file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp *.ixx)
set(SOURCES ${SOURCES})

# build target graph (library)
add_library(graph ${SOURCES})
target_link_libraries(graph PRIVATE spdlog::spdlog)

set_default_warnings(graph)

enable_sanitizers(graph
  "${ENABLE_SANITIZER_ADDRESS}"
  "${ENABLE_SANITIZER_LEAK}"
  "${ENABLE_SANITIZER_UNDEFINED_BEHAVIOR}"
  "${ENABLE_SANITIZER_THREAD}"
  "${ENABLE_SANITIZER_MEMORY}")
