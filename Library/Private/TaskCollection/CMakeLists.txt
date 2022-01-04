sober_library_begin (TaskCollection STATIC)
    file (GLOB_RECURSE HEADERS *.hpp)
    file (GLOB_RECURSE SOURCES *.cpp)
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

    sober_library_use_service (EXPECTED PUBLIC Memory)

    sober_naming_variant_target (Container Original CONTAINER_LIBRARY_TARGET)
    sober_library_link_library (PUBLIC ${CONTAINER_LIBRARY_TARGET})

    sober_variant_begin (Original)
    sober_variant_end ()
sober_library_end ()
