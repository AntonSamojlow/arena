# cmake file for src/sag


# external dependencies
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
set(DEPENDENCIES spdlog::spdlog fmt::fmt unofficial::sqlite3::sqlite3)

# source files
file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp *.ixx)
set(SOURCES ${SOURCES})

# build target sag (library)
add_library(sag ${SOURCES})
target_link_libraries(sag PUBLIC ${DEPENDENCIES})
set_target_properties(sag PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(sag PRIVATE "")
get_property(include_dirs TARGET sag PROPERTY INCLUDE_DIRECTORIES)
message(STATUS "[sag] include directories:'${include_dirs}'")

set_default_warnings(sag)

enable_sanitizers(sag
  "${ENABLE_SANITIZER_ADDRESS}"
  "${ENABLE_SANITIZER_LEAK}"
  "${ENABLE_SANITIZER_UNDEFINED_BEHAVIOR}"
  "${ENABLE_SANITIZER_THREAD}"
  "${ENABLE_SANITIZER_MEMORY}")

enable_coverage(sag)
