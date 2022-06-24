sober_library_begin ("${GAME_NAME}ExtensionTest" STATIC)
    file (GLOB_RECURSE HEADERS *.hpp)
    file (GLOB_RECURSE SOURCES *.cpp)
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

    sober_naming_variant_target ("${GAME_NAME}Extension" Original EXTENSION_TARGET)
    sober_naming_variant_target (MemoryProfilerStub Original MEMORY_PROFILER_STUB_TARGET)

    sober_library_link_library (PUBLIC ${EXTENSION_TARGET})
    sober_library_link_library (PRIVATE ${MEMORY_PROFILER_STUB_TARGET})

    sober_library_use_service (PER_VARIANT PRIVATE Log)
    sober_library_use_service (PER_VARIANT PRIVATE Memory)
    sober_library_use_service (PER_VARIANT PRIVATE MemoryProfiler)
    sober_library_use_service (PER_VARIANT PRIVATE RecordCollection)
    sober_library_use_service (PER_VARIANT PRIVATE StandardLayoutMapping)
    sober_library_use_service (SHARED PRIVATE Testing)

    # On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
    if (MSVC)
        sober_library_private_compile_options (/EHs)
    endif ()

    sober_variant_begin (Original)
    sober_variant_end ()
sober_library_end ()

add_executable ("${GAME_NAME}ExtensionTestLauncher" "${SOBER_TEST_RUNNER_STUB_SOURCE}")
sober_naming_variant_target ("${GAME_NAME}ExtensionTest" Original LIBRARY_TARGET)
target_link_libraries ("${GAME_NAME}ExtensionTestLauncher" PUBLIC "${LIBRARY_TARGET}")

set (RUNNER_OUTPUT_DIRECTORY "${GAME_OUTPUT_DIRECTORY}/Test/Extension")
set_target_properties ("${GAME_NAME}ExtensionTestLauncher" PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${RUNNER_OUTPUT_DIRECTORY}")
copy_required_shared_libraries ("${GAME_NAME}ExtensionTestLauncher")

add_test (NAME "Test${GAME_NAME}ExtensionRunner" COMMAND "${GAME_NAME}ExtensionTestLauncher"
        WORKING_DIRECTORY "${RUNNER_OUTPUT_DIRECTORY}")
add_dependencies (EmergenceTests "${GAME_NAME}ExtensionTestLauncher")
