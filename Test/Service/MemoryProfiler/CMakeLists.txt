# We have two sets of tests:
# - Tests for really implemented profiling, that check if it works correctly.
# - Tests for stubbed out profiling, that check if it has expected safe behaviour.

# Test for real implementations.
sober_test_begin (MemoryProfiler)
    file (GLOB_RECURSE SOURCES "Implemented/*.cpp")
    file (GLOB_RECURSE HEADERS "Implemented/*.hpp")
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Implemented")

    sober_naming_variant_target (Container Original CONTAINER_LIBRARY_TARGET)
    sober_naming_variant_target (MemoryProfilerStub Original MEMORY_PROFILER_STUB_LIBRARY_TARGET)

    sober_library_link_library (PRIVATE "${CONTAINER_LIBRARY_TARGET}")
    sober_library_link_library (PRIVATE ${MEMORY_PROFILER_STUB_LIBRARY_TARGET})

    sober_library_use_service (SHARED PRIVATE Memory Original FROZEN)
    sober_library_use_service (SHARED PRIVATE Testing)

    # On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
    if (MSVC)
        sober_library_private_compile_options (/EHs)
    endif ()

    sober_test_variant_begin (Original)
    sober_test_variant_end ()
sober_test_end ()

sober_naming_test_collection (MemoryProfiler COLLECTION_NAME)
add_dependencies (ServiceTests "${COLLECTION_NAME}")

# Test for stub implementation.
file (GLOB_RECURSE SOURCES "Stub/*.cpp")
file (GLOB_RECURSE HEADERS "Stub/*.hpp")

add_executable (TestMemoryProfilerNone ${SOURCES} ${HEADERS})
add_test (NAME "TestMemoryProfilerNone" COMMAND TestMemoryProfilerNone)
add_dependencies (ServiceTests TestMemoryProfilerNone)
target_include_directories (TestMemoryProfilerNone PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Stub")

sober_naming_implementation_target (Memory Original MEMORY_IMPLEMENTATION_TARGET)
sober_naming_service_target (MemoryProfiler MEMORY_PROFILER_TARGET)
sober_naming_implementation_target (MemoryProfiler None MEMORY_PROFILER_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (Testing Doctest TESTING_LIBRARY_TARGET)

target_link_libraries (TestMemoryProfilerNone PRIVATE
        ${MEMORY_IMPLEMENTATION_TARGET} ${MEMORY_PROFILER_TARGET}
        ${MEMORY_PROFILER_IMPLEMENTATION_TARGET} ${TESTING_LIBRARY_TARGET})

# On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
if (MSVC)
    target_compile_options (TestMemoryProfilerNone PRIVATE /EHa)
endif ()
