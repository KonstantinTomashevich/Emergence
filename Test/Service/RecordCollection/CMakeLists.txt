sober_test_begin (RecordCollection)
    file (GLOB_RECURSE SOURCES *.cpp)
    file (GLOB_RECURSE HEADERS *.hpp)
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

    sober_naming_variant_target (ContextExtension Original CONTEXT_EXTENSION_LIBRARY_TARGET)
    sober_library_link_library (PRIVATE ${CONTEXT_EXTENSION_LIBRARY_TARGET})

    sober_naming_variant_target (RecordCollectionVisualization Original VISUALIZATION_LIBRARY_TARGET)
    sober_library_link_library (PRIVATE ${VISUALIZATION_LIBRARY_TARGET})

    sober_naming_variant_target (TestQueryAPI Original TEST_QUERY_API_LIBRARY_TARGET)
    sober_library_link_library (PRIVATE ${TEST_QUERY_API_LIBRARY_TARGET})

    sober_naming_variant_target (TestReferenceAPI Original TEST_REFERENCE_API_LIBRARY_TARGET)
    sober_library_link_library (PRIVATE ${TEST_REFERENCE_API_LIBRARY_TARGET})

    # On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
    if (MSVC)
        sober_library_private_compile_options (/EHa)
    endif ()

    sober_test_variant_begin (PegasusOriginal)
    sober_test_variant_end ()
sober_test_end ()

sober_naming_test_collection (RecordCollection COLLECTION_NAME)
add_dependencies (ServiceTests "${COLLECTION_NAME}")
