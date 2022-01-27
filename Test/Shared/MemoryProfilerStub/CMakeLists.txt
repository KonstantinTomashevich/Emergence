sober_library_begin (MemoryProfilerStub INTERFACE)
    file (GLOB_RECURSE HEADERS *.hpp)

    sober_library_set_sources ("${HEADERS}")
    sober_library_include_directory (INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")

    sober_variant_begin (Original)
    sober_variant_end ()
sober_library_end ()
