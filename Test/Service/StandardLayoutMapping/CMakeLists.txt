sober_test_begin (StandardLayoutMapping)
    file (GLOB_RECURSE SOURCES *.cpp)
    file (GLOB_RECURSE HEADERS *.hpp)
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

    # Testing backend requires implementation headers, therefore it's usage as service would trigger separate
    # compilation for all variants. But there is no sense to use different testing backends for each variant,
    # therefore we just link it explicitly.
    sober_naming_variant_target (Testing Doctest TESTING_LIBRARY_TARGET)
    sober_library_link_library (PRIVATE ${TESTING_LIBRARY_TARGET})

    # On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
    if (MSVC)
        sober_library_private_compile_options (/EHa)
    endif ()

    sober_test_variant_begin (Original)
    sober_test_variant_end ()
sober_test_end ()

sober_naming_test_collection (StandardLayoutMapping COLLECTION_NAME)
add_dependencies (ServiceTests "${COLLECTION_NAME}")
