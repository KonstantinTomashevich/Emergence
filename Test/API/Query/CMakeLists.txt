sober_library_begin (TestQueryAPI STATIC)
    file (GLOB_RECURSE HEADERS *.hpp)
    file (GLOB_RECURSE SOURCES *.cpp)
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

    sober_naming_variant_target (ContextExtension Original CONTEXT_EXTENSION_LIBRARY_TARGET)
    sober_library_link_library (PUBLIC ${CONTEXT_EXTENSION_LIBRARY_TARGET})

    sober_naming_variant_target (SyntaxSugar Original SYNTAX_SUGAR_LIBRARY_TARGET)
    sober_library_link_library (PUBLIC ${SYNTAX_SUGAR_LIBRARY_TARGET})

    sober_library_use_service (PUBLIC StandardLayoutMapping)
    sober_library_use_service (PUBLIC Testing)

    # On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
    if (MSVC)
        sober_library_private_compile_options (/EHs)
    endif ()

    sober_variant_begin (Original)
    sober_variant_end ()
sober_library_end ()
