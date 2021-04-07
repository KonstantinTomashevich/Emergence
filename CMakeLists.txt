cmake_minimum_required (VERSION 3.17)
project (Emergence)

set (CMAKE_CXX_STANDARD 17)
include ("${CMAKE_SOURCE_DIR}/CMake/Sober/Sober.cmake")

add_subdirectory (Service)
add_subdirectory (Library)
add_subdirectory (Test)
add_subdirectory (Executable)