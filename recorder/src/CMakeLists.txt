# CMakeList.txt : CMake project for recorder, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (recorder "recorder.cpp" "recorder.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET recorder PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
