cmake_minimum_required (VERSION 3.21)
cmake_policy (VERSION 3.21)

project (Emergence)
set (CMAKE_CXX_STANDARD 20)

include ("${CMAKE_SOURCE_DIR}/CMake/Emergence/GlobalOptions.cmake")
include ("${CMAKE_SOURCE_DIR}/CMake/Emergence/PhysX.cmake")
include ("${CMAKE_SOURCE_DIR}/CMake/Emergence/Urho3D.cmake")
include ("${CMAKE_SOURCE_DIR}/CMake/Emergence/Utility.cmake")
include ("${CMAKE_SOURCE_DIR}/CMake/Sober/Sober.cmake")

# Must be called from root CMakeLists.txt, otherwise CTest configuration won't be placed into binary root.
enable_testing ()

add_subdirectory (Service)
add_subdirectory (Library)
add_subdirectory (Test)
add_subdirectory (ThirdParty)
add_subdirectory (Executable)
