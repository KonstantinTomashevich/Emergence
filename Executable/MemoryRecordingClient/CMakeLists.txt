sober_library_begin (MemoryRecordingClient STATIC)
    file (GLOB_RECURSE HEADERS *.hpp)
    file (GLOB_RECURSE SOURCES *.cpp)
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

    sober_naming_variant_target (Container Original CONTAINER_LIBRARY_TARGET)
    sober_naming_variant_target (MemoryRecording Original MEMORY_RECORDING_LIBRARY_TARGET)

    sober_library_link_library (PRIVATE ${CONTAINER_LIBRARY_TARGET})
    sober_library_link_library (PRIVATE ${MEMORY_RECORDING_LIBRARY_TARGET})
    sober_library_link_library (PRIVATE SDL2)
    sober_library_link_library (PRIVATE SDL2ImGUI)
    sober_library_link_library (PRIVATE SDL2main)

    sober_library_use_service (SHARED PRIVATE Log)
    sober_library_use_service (SHARED PRIVATE Memory)
    sober_library_use_service (SHARED PRIVATE MemoryProfiler)
    sober_library_use_service (SHARED PRIVATE StandardLayoutMapping)

    sober_variant_begin (Original)
    sober_variant_end ()
sober_library_end ()

add_executable (MemoryRecordingClientLauncher "${SOBER_TEST_RUNNER_STUB_SOURCE}")
sober_naming_variant_target (MemoryRecordingClient Original CLIENT_TARGET)
target_link_libraries (MemoryRecordingClientLauncher PUBLIC "${CLIENT_TARGET}")

set_target_properties (MemoryRecordingClientLauncher PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Tools/MemoryRecordingClient")
copy_required_shared_libraries (MemoryRecordingClientLauncher)
