# CMake project file
set(TARGET_NAME recorder)

# prepare external dependencies
find_package(spdlog REQUIRED)
set(DEPENDENCIES spdlog::spdlog app)

# source files
file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp *.ixx)
set(SOURCES ${SOURCES})

# build target
add_executable(${TARGET_NAME} ${SOURCES})
target_link_libraries(${TARGET_NAME} PUBLIC ${DEPENDENCIES})
set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)

# log include directories
get_property(include_dirs TARGET ${TARGET_NAME} PROPERTY INCLUDE_DIRECTORIES)
message(STATUS "[${TARGET_NAME}] include directories:'${include_dirs}'")

# run desired cmake helper scripts
set_default_warnings(${TARGET_NAME})
enable_configured_sanitizers(${TARGET_NAME})
enable_coverage(${TARGET_NAME})
