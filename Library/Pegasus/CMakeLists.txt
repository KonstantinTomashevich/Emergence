add_subdirectory (Constants)

sober_library_begin (Pegasus STATIC)
    file (GLOB_RECURSE HEADERS *.hpp)
    file (GLOB_RECURSE SOURCES *.cpp)
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Public")
    sober_library_include_directory (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Private")

    sober_library_use_service (Hashing PRIVATE)
    sober_library_use_service (MemoryPool PUBLIC)
    sober_library_use_service (PegasusConstants PUBLIC)
    sober_library_use_service (StandardLayoutMapping PUBLIC)

    sober_naming_variant_target (APICommon Original API_COMMON_LIBRARY_TARGET)
    sober_library_link_library (PUBLIC ${API_COMMON_LIBRARY_TARGET})

    sober_naming_variant_target (Handling Original HANDLING_LIBRARY_TARGET)
    sober_library_link_library (PUBLIC ${HANDLING_LIBRARY_TARGET})

    sober_naming_variant_target (SyntaxSugar Original SYNTAX_SUGAR_LIBRARY_TARGET)
    sober_library_link_library (PUBLIC ${SYNTAX_SUGAR_LIBRARY_TARGET})

    sober_variant_begin (Original)
        sober_variant_freeze_implementation (PegasusConstants Original)
    sober_variant_end ()
sober_library_end ()
