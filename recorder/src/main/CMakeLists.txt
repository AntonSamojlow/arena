# cmake file for src/main

# external dependencies
find_package(spdlog REQUIRED)
set(DEPENDENCIES spdlog::spdlog mylibrary)

# source files
file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp *.ixx)
set(SOURCES ${SOURCES})

# build target main (executable)
add_executable(main ${SOURCES})
target_link_libraries(main PRIVATE ${DEPENDENCIES})

set_default_warnings(main)

enable_sanitizers(main
  "${ENABLE_SANITIZER_ADDRESS}"
  "${ENABLE_SANITIZER_LEAK}"
  "${ENABLE_SANITIZER_UNDEFINED_BEHAVIOR}"
  "${ENABLE_SANITIZER_THREAD}"
  "${ENABLE_SANITIZER_MEMORY}")
