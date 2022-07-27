sober_library_begin (Serialization STATIC)
    file (GLOB_RECURSE HEADERS *.hpp)
    file (GLOB_RECURSE SOURCES *.cpp)
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

    sober_naming_variant_target (SyntaxSugar Original SYNTAX_SUGAR_LIBRARY_TARGET)

    sober_library_link_library (PRIVATE "${SYNTAX_SUGAR_LIBRARY_TARGET}")
    sober_library_link_library (PRIVATE yaml-cpp)

    sober_library_use_service (EXPECTED PRIVATE Log)
    sober_library_use_service (EXPECTED PUBLIC StandardLayoutMapping)

    # On MSVC CLang exceptions are disabled by default,
    # but we need to compile with exceptions because of yaml-cpp templates.
    if (MSVC)
        sober_library_private_compile_options (/EHa)
    endif ()

    sober_variant_begin (Original)
    sober_variant_end ()
sober_library_end ()
