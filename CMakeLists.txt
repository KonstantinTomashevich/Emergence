cmake_minimum_required (VERSION 3.21)
cmake_policy (VERSION 3.21)

project (Emergence)
# Due to the issues with CLang in VS 2022, we need to run coverage on VS 2019, therefore this flag is needed.
option (EMERGENCE_FALLBACK_TO_CXX_20 "Enables deprecated C++ 20 support." OFF)

if (EMERGENCE_FALLBACK_TO_CXX_20)
    set (CMAKE_CXX_STANDARD 20)
    add_compile_definitions ("EMERGENCE_FALLBACK_TO_CXX_20")
else ()
    set (CMAKE_CXX_STANDARD 23)
endif ()

include ("${CMAKE_CURRENT_SOURCE_DIR}/CMake/Emergence/BuildSystem.cmake")
include ("${CMAKE_CURRENT_SOURCE_DIR}/CMake/Emergence/CodeGeneration.cmake")
include ("${CMAKE_CURRENT_SOURCE_DIR}/CMake/Emergence/GlobalOptions.cmake")
include ("${CMAKE_CURRENT_SOURCE_DIR}/CMake/Emergence/PhysX.cmake")
include ("${CMAKE_CURRENT_SOURCE_DIR}/CMake/Emergence/Resource.cmake")

# Must be called from root CMakeLists.txt, otherwise CTest configuration won't be placed into binary root.
enable_testing ()

# We need this target to generate code for CheckCodeStyle workflow without actually building all projects.
add_custom_target (BuildTimeCodeGeneration COMMENT "Generate all code that is usually generated during build process.")

add_subdirectory (ThirdParty)
add_subdirectory (Unit)
add_subdirectory (Test)
add_subdirectory (Executable)
