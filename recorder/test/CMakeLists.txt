# CMake *test* project file
set(TARGET_NAME tests)

# prepare external dependencies
find_package(Catch2 REQUIRED)
find_package(spdlog REQUIRED)
set(DEPENDENCIES spdlog::spdlog fmt::fmt unofficial::sqlite3::sqlite3)

# source files
file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp *.ixx)
set(SOURCES ${SOURCES})
set(LIBRARIESUNDERTEST sag tools)

# build target test (executable)
add_executable(tests ${SOURCES})
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain spdlog::spdlog)
target_link_libraries(tests PRIVATE ${LIBRARIESUNDERTEST})

# log include directories
get_property(include_dirs TARGET ${TARGET_NAME} PROPERTY INCLUDE_DIRECTORIES)
message(STATUS "[${TARGET_NAME}] include directories:'${include_dirs}'")

# run desired cmake helper scripts
set_test_warnings(${TARGET_NAME})
enable_configured_sanitizers(${TARGET_NAME})
enable_coverage(${TARGET_NAME})

# use catch2 script to autodiscover all tests
include(CTest)
include(Catch)
catch_discover_tests(tests)

