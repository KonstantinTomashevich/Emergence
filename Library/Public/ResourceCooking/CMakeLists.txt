sober_library_begin (ResourceCooking OBJECT)
    file (GLOB_RECURSE HEADERS "*.hpp")
    file (GLOB_RECURSE SOURCES "*.cpp")
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

    sober_naming_variant_target (Container Original CONTAINER_ORIGINAL_LIBRARY)
    sober_naming_variant_target (Serialization Original SERIALIZATION_ORIGINAL_LIBRARY)
    sober_naming_variant_target (SyntaxSugar Original SYNTAX_SUGAR_ORIGINAL_LIBRARY)

    sober_library_link_library (PRIVATE "${SERIALIZATION_ORIGINAL_LIBRARY}")
    sober_library_link_library (PUBLIC "${CONTAINER_ORIGINAL_LIBRARY}")
    sober_library_link_library (PUBLIC "${SYNTAX_SUGAR_ORIGINAL_LIBRARY}")

    sober_library_use_service (EXPECTED PRIVATE JobDispatcher)
    sober_library_use_service (EXPECTED PRIVATE Log)
    sober_library_use_service (EXPECTED PUBLIC RecordCollection)
    sober_library_use_service (EXPECTED PUBLIC ResourceProvider)
    sober_library_use_service (EXPECTED PUBLIC StandardLayoutMapping)

    sober_variant_begin (Original)
    sober_variant_end ()
sober_library_end ()
