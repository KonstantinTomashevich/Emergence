add_subdirectory (Constants)

sober_library_begin (Pegasus STATIC)
    file (GLOB_RECURSE HEADERS "Public/*.hpp" "Private/*.hpp")
    file (GLOB_RECURSE SOURCES "Public/*.cpp" "Private/*.cpp")
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Public")
    sober_library_include_directory (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Private")

    sober_library_use_service (EXPECTED PUBLIC StandardLayoutMapping)
    sober_library_use_service (EXPECTED PUBLIC Memory)
    sober_library_use_service (EXPECTED PUBLIC MemoryProfiler)
    sober_library_use_service (PER_VARIANT PRIVATE Hashing)
    sober_library_use_service (PER_VARIANT PUBLIC PegasusConstants)

    sober_naming_variant_target (APICommon Original API_COMMON_LIBRARY_TARGET)
    sober_library_link_library (PUBLIC ${API_COMMON_LIBRARY_TARGET})

    sober_naming_variant_target (Container Original CONTAINER_LIBRARY_TARGET)
    sober_library_link_library (PUBLIC "${CONTAINER_LIBRARY_TARGET}")

    sober_naming_variant_target (Handling Original HANDLING_LIBRARY_TARGET)
    sober_library_link_library (PUBLIC ${HANDLING_LIBRARY_TARGET})

    sober_naming_variant_target (SyntaxSugar Original SYNTAX_SUGAR_LIBRARY_TARGET)
    sober_library_link_library (PUBLIC ${SYNTAX_SUGAR_LIBRARY_TARGET})

    sober_variant_begin (Original)
        sober_variant_freeze_implementation (PegasusConstants Original)
    sober_variant_end ()
sober_library_end ()
