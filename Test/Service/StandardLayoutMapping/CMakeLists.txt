sober_test_begin (StandardLayoutMapping)
    file (GLOB_RECURSE SOURCES *.cpp)
    file (GLOB_RECURSE HEADERS *.hpp)
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

    sober_naming_variant_target (StandardLayoutMappingVisualization Original VISUALIZATION_LIBRARY_TARGET)
    sober_library_link_library (PRIVATE ${VISUALIZATION_LIBRARY_TARGET})

    sober_naming_variant_target (SyntaxSugar Original SYNTAX_SUGAR_LIBRARY_TARGET)
    sober_library_link_library (PRIVATE ${SYNTAX_SUGAR_LIBRARY_TARGET})

    sober_library_use_shared_service (PRIVATE Testing Doctest)

    # On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
    if (MSVC)
        sober_library_private_compile_options (/EHs)
    endif ()

    sober_test_variant_begin (Original)
    sober_test_variant_end ()
sober_test_end ()

sober_naming_test_collection (StandardLayoutMapping COLLECTION_NAME)
add_dependencies (ServiceTests "${COLLECTION_NAME}")
