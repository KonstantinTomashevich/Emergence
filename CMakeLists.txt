cmake_minimum_required (VERSION 3.21)
cmake_policy (VERSION 3.21)

project (Emergence)
set (CMAKE_CXX_STANDARD 23)

include ("${CMAKE_CURRENT_SOURCE_DIR}/CMake/Emergence/GlobalOptions.cmake")
include ("${CMAKE_CURRENT_SOURCE_DIR}/CMake/Emergence/PhysX.cmake")
include ("${CMAKE_CURRENT_SOURCE_DIR}/CMake/Emergence/Resource.cmake")
include ("${CMAKE_CURRENT_SOURCE_DIR}/CMake/Emergence/Urho3D.cmake")
include ("${CMAKE_CURRENT_SOURCE_DIR}/CMake/Emergence/Utility.cmake")
include ("${CMAKE_CURRENT_SOURCE_DIR}/CMake/Sober/Sober.cmake")

# Must be called from root CMakeLists.txt, otherwise CTest configuration won't be placed into binary root.
enable_testing ()

# We need this target to generate code for CheckCodeStyle workflow without actually building all projects.
add_custom_target (BuildTimeCodeGeneration COMMENT "Generate all code that is usually generated during build process.")

add_subdirectory (ThirdParty)
add_subdirectory (Service)
add_subdirectory (Library)
add_subdirectory (Test)
add_subdirectory (Executable)
