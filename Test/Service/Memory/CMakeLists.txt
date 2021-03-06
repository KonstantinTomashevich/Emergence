sober_test_begin (Memory)
    file (GLOB_RECURSE SOURCES *.cpp)
    file (GLOB_RECURSE HEADERS *.hpp)
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

    sober_naming_variant_target (Container Original CONTAINER_LIBRARY_TARGET)
    sober_naming_variant_target (MemoryProfilerStub Original MEMORY_PROFILER_STUB_LIBRARY_TARGET)

    sober_library_link_library (PUBLIC ${CONTAINER_LIBRARY_TARGET})
    sober_library_link_library (PUBLIC ${MEMORY_PROFILER_STUB_LIBRARY_TARGET})

    sober_library_use_service (SHARED PRIVATE MemoryProfiler Original FROZEN)
    sober_library_use_service (SHARED PRIVATE StandardLayoutMapping)
    sober_library_use_service (SHARED PRIVATE Testing)

    # On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
    if (MSVC)
        sober_library_private_compile_options (/EHs)
    endif ()

    sober_test_variant_begin (Original)
    sober_test_variant_end ()
sober_test_end ()

sober_naming_test_collection (Memory COLLECTION_NAME)
add_dependencies (ServiceTests "${COLLECTION_NAME}")
