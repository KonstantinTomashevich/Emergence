sober_library_begin (ContextExtension INTERFACE)
    file (GLOB_RECURSE HEADERS *.hpp)

    sober_library_set_sources ("${HEADERS}")
    sober_library_include_directory (INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")

    sober_naming_variant_target (Container Original CONTAINER_LIBRARY_TARGET)
    sober_naming_variant_target (MemoryProfilerStub Original MEMORY_PROFILER_STUB_LIBRARY_TARGET)

    sober_library_link_library (INTERFACE "${CONTAINER_LIBRARY_TARGET}")
    sober_library_link_library (INTERFACE ${MEMORY_PROFILER_STUB_LIBRARY_TARGET})

    sober_library_use_service (EXPECTED INTERFACE Testing)
    sober_variant_begin (Original)
    sober_variant_end ()
sober_library_end ()
