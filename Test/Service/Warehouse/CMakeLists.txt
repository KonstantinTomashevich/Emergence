add_subdirectory (Visualization)

sober_test_begin (Warehouse OBJECT)
    file (GLOB_RECURSE SOURCES "Shared/*.cpp")
    file (GLOB_RECURSE HEADERS "Shared/*.hpp")
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Shared")

    sober_naming_variant_target (ContextExtension Original CONTEXT_EXTENSION_LIBRARY_TARGET)
    sober_library_link_library (PRIVATE ${CONTEXT_EXTENSION_LIBRARY_TARGET})

    sober_naming_variant_target (TestQueryAPI Original TEST_QUERY_API_LIBRARY_TARGET)
    sober_library_link_library (PRIVATE ${TEST_QUERY_API_LIBRARY_TARGET})

    sober_naming_variant_target (TestReferenceAPI Original TEST_REFERENCE_API_LIBRARY_TARGET)
    sober_library_link_library (PRIVATE ${TEST_REFERENCE_API_LIBRARY_TARGET})

    sober_naming_variant_target (WarehouseVisualization Original VISUALIZATION_LIBRARY_TARGET)
    sober_library_link_library (PRIVATE ${VISUALIZATION_LIBRARY_TARGET})

    sober_library_use_service (PER_VARIANT PRIVATE TestWarehouseVisualizationGraphs)
    sober_library_use_service (SHARED PRIVATE Assert)
    sober_library_use_service (SHARED PRIVATE CPUProfiler)
    sober_library_use_service (SHARED PRIVATE Log)
    sober_library_use_service (SHARED PRIVATE Memory)
    sober_library_use_service (SHARED PRIVATE MemoryProfiler)
    sober_library_use_service (SHARED PRIVATE RecordCollection)
    sober_library_use_service (SHARED PRIVATE StandardLayoutMapping)

    if (MSVC)
        # On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
        sober_library_private_compile_options (/EHs)

        # Scenario object file is too big for MSVC default configuration, therefore we need to increase object size.
        sober_library_private_compile_options (/bigobj)
    endif ()

    sober_test_variant_begin (GalleonOriginal)
        sober_variant_freeze_implementation (WarehouseVisualizationGraphs Galleon)
    sober_test_variant_end ()
sober_test_end ()

sober_naming_test_collection (Warehouse COLLECTION_NAME)
add_dependencies (ServiceTests "${COLLECTION_NAME}")
finish_test_setup ()
