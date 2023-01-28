# cmake file for test

# external dependencies
find_package(Catch2 REQUIRED)
find_package(spdlog REQUIRED)

# source files
file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp *.ixx)
set(SOURCES ${SOURCES})
set(LIBRARIESUNDERTEST sag)

# build target test (executable)
add_executable(tests ${SOURCES})
# main is provided by catch:
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain spdlog::spdlog)
# link the code to be tested:
target_link_libraries(tests PRIVATE ${LIBRARIESUNDERTEST})

set_default_warnings(tests)

enable_sanitizers(tests
  "${ENABLE_SANITIZER_ADDRESS}"
  "${ENABLE_SANITIZER_LEAK}"
  "${ENABLE_SANITIZER_UNDEFINED_BEHAVIOR}"
  "${ENABLE_SANITIZER_THREAD}"
  "${ENABLE_SANITIZER_MEMORY}")

# use catch2 script to autodiscover all tests
include(CTest)
include(Catch)
catch_discover_tests(tests)

enable_coverage(tests)
