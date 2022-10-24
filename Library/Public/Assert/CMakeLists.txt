sober_library_begin (Assert STATIC)
    file (GLOB_RECURSE HEADERS "*.hpp")
    file (GLOB_RECURSE SOURCES "*.cpp")
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

    sober_naming_variant_target (SyntaxSugar Original SYNTAX_SUGAR_ORIGINAL_LIBRARY)

    sober_library_link_library (PRIVATE DebugBreak)
    sober_library_link_library (PRIVATE SDL2)
    sober_library_link_library (PUBLIC "${SYNTAX_SUGAR_ORIGINAL_LIBRARY}")

    sober_library_use_service (EXPECTED PRIVATE Log)

    sober_variant_begin (Original)
    sober_variant_end ()
sober_library_end ()
