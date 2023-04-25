# CMake project file
set(TARGET_NAME app)

# prepare external dependencies
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
set(DEPENDENCIES spdlog::spdlog fmt::fmt unofficial::sqlite3::sqlite3 nlohmann_json::nlohmann_json sag tools)

# source files
file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp *.ixx)
set(SOURCES ${SOURCES})

# build target
add_library(${TARGET_NAME} ${SOURCES})
target_link_libraries(${TARGET_NAME} PUBLIC ${DEPENDENCIES})
set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)

# log include directories
get_property(include_dirs TARGET ${TARGET_NAME} PROPERTY INCLUDE_DIRECTORIES)
message(STATUS "[${TARGET_NAME}] include directories:'${include_dirs}'")

# run desired cmake helper scripts
set_default_warnings(${TARGET_NAME})
enable_configured_sanitizers(${TARGET_NAME})
enable_coverage(${TARGET_NAME})
