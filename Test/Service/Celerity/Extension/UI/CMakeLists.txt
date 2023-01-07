if (NOT EMERGENCE_INCLUDE_GPU_DEPENDANT_TESTS)
    return ()
endif ()

add_subdirectory (ImplementationStrings)
sober_test_begin (CelerityUI)
    file (GLOB_RECURSE SOURCES "Private/*.cpp")
    file (GLOB_RECURSE HEADERS "Private/*.hpp")
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Private")

    sober_naming_variant_target (CelerityRenderFoundation Original CELERITY_RENDER_FOUNDATION_LIBRARY_TARGET)
    sober_naming_variant_target (MemoryProfilerStub Original MEMORY_PROFILER_STUB_LIBRARY_TARGET)

    sober_library_link_library (PUBLIC ${CELERITY_RENDER_FOUNDATION_LIBRARY_TARGET})
    sober_library_link_library (PUBLIC ${MEMORY_PROFILER_STUB_LIBRARY_TARGET})
    sober_library_link_library (PUBLIC SDL2)
    sober_library_link_library (PUBLIC SDL2main)

    sober_library_use_service (PER_VARIANT PRIVATE TestCelerityUIImplementationStrings)
    sober_library_use_service (SHARED PRIVATE Assert)
    sober_library_use_service (SHARED PRIVATE JobDispatcher)
    sober_library_use_service (SHARED PRIVATE Log)
    sober_library_use_service (SHARED PRIVATE Memory)
    sober_library_use_service (SHARED PRIVATE MemoryProfiler)
    sober_library_use_service (SHARED PRIVATE RecordCollection)
    sober_library_use_service (SHARED PRIVATE RenderBackend)
    sober_library_use_service (SHARED PRIVATE StandardLayoutMapping)
    sober_library_use_service (SHARED PRIVATE TaskExecutor)
    sober_library_use_service (SHARED PRIVATE Testing)
    sober_library_use_service (SHARED PRIVATE Warehouse)

    # On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
    if (MSVC)
        sober_library_private_compile_options (/EHs)
    endif ()

    sober_test_variant_begin (ImGUI)
        sober_variant_freeze_implementation (TestCelerityUIImplementationStrings ImGUI)
    sober_test_variant_end ()
sober_test_end ()

sober_naming_test_collection (CelerityUI COLLECTION_NAME)
add_dependencies (ServiceTests "${COLLECTION_NAME}")
finish_test_setup ()
