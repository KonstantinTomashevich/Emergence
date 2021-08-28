sober_library_begin (ContextExtension INTERFACE)
    file (GLOB_RECURSE HEADERS *.hpp)

    sober_library_set_sources ("${HEADERS}")
    sober_library_include_directory (INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")

    sober_library_use_service (Testing INTERFACE)
    sober_variant_begin (Original)
    sober_variant_end ()
sober_library_end ()
