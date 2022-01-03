sober_library_begin (Galleon STATIC)
    file (GLOB_RECURSE HEADERS *.hpp)
    file (GLOB_RECURSE SOURCES *.cpp)
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

    sober_library_use_service (EXPECTED PUBLIC StandardLayoutMapping)
    sober_library_use_service (EXPECTED PUBLIC Memory)
    sober_library_use_service (EXPECTED PUBLIC MemoryProfiler)
    sober_library_use_service (EXPECTED PUBLIC RecordCollection)

    sober_naming_variant_target (Container Original CONTAINER_LIBRARY_TARGET)
    sober_library_link_library (PUBLIC ${CONTAINER_LIBRARY_TARGET})

    sober_naming_variant_target (Handling Original HANDLING_LIBRARY_TARGET)
    sober_library_link_library (PUBLIC ${HANDLING_LIBRARY_TARGET})

    sober_variant_begin (Original)
    sober_variant_end ()
sober_library_end ()
