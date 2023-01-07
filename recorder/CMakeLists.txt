cmake_minimum_required (VERSION 3.24)

project ("recorder"
  VERSION 0.0.1
  DESCRIPTION ""
  HOMEPAGE_URL "https://github.com/AntonSamojlow/arena"
  LANGUAGES CXX C
)

# set global c++ standard to c++20
set(CMAKE_CXX_STANDARD 20)

# log base compiler info
if(CMAKE_CXX_COMPILER_LOADED)
  message(STATUS "CMAKE_CXX_COMPILER='${CMAKE_CXX_COMPILER}'")
  message(STATUS "CMAKE_CXX_COMPILER_ID='${CMAKE_CXX_COMPILER_ID}'")
  message(STATUS "CMAKE_CXX_COMPILER_VERSION='${CMAKE_CXX_COMPILER_VERSION}'")
  message(STATUS "CMAKE_COMPILER_IS_GNUCXX='${CMAKE_COMPILER_IS_GNUCXX}'")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/cmake/Coverage.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/Sanitizers.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/StaticAnalysis.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/Warnings.cmake")

# allows test and other libs to reference src folder directly 
# (else all cross-project includes must start with 'src/' or '../')
include_directories(src)

if(ENABLE_CLANG_TIDY)
  enable_clang_tidy()
endif()

# include sub-projects
add_subdirectory (src/recorder)
#add_subdirectory (src/graph)

# set up testing
enable_testing()
# include test projects
add_subdirectory (test)

