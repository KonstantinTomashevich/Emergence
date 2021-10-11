add_subdirectory (Expectation)

sober_test_begin (ExportGraph)
    file (GLOB_RECURSE SOURCES "Shared/*.cpp")
    file (GLOB_RECURSE HEADERS "Shared/*.hpp")
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Shared")

    sober_library_use_shared_service (PRIVATE Testing Doctest)
    sober_library_use_service (PRIVATE TestExportGraphExpectation)

    # On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
    if (MSVC)
        sober_library_private_compile_options (/EHs)
    endif ()

    sober_test_variant_begin (DOT)
        sober_variant_freeze_implementation (TestExportGraphExpectation DOT)
    sober_test_variant_end ()
sober_test_end ()

sober_naming_test_collection (ExportGraph COLLECTION_NAME)
add_dependencies (ServiceTests "${COLLECTION_NAME}")
