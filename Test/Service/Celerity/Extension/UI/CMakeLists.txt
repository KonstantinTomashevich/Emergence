if (NOT EMERGENCE_INCLUDE_GPU_DEPENDANT_TESTS)
    return ()
endif ()

add_subdirectory (ImplementationStrings)
sober_test_begin (CelerityUI OBJECT)
    file (GLOB_RECURSE SOURCES "Private/*.cpp")
    file (GLOB_RECURSE HEADERS "Private/*.hpp")
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Private")

    sober_naming_variant_target (CelerityLocale Original CELERITY_LOCALE_LIBRARY_TARGET)
    sober_naming_variant_target (CelerityRenderFoundation Original CELERITY_RENDER_FOUNDATION_LIBRARY_TARGET)
    sober_naming_variant_target (FileSystemTestUtility Doctest FILE_SYSTEM_TEST_UTILITY_LIBRARY_TARGET)
    sober_naming_variant_target (MemoryProfilerStub Original MEMORY_PROFILER_STUB_LIBRARY_TARGET)
    sober_naming_variant_target (ResourceContextHolder Doctest RESOURCE_CONTEXT_HOLDER_LIBRARY_TARGET)
    sober_naming_variant_target (SDLContextHolder Original SDL_CONTEXT_HOLDER_LIBRARY_TARGET)
    sober_naming_variant_target (Serialization Original SERIALIZATION_LIBRARY_TARGET)

    sober_library_link_library (PUBLIC ${CELERITY_LOCALE_LIBRARY_TARGET})
    sober_library_link_library (PUBLIC ${CELERITY_RENDER_FOUNDATION_LIBRARY_TARGET})
    sober_library_link_library (PUBLIC ${FILE_SYSTEM_TEST_UTILITY_LIBRARY_TARGET})
    sober_library_link_library (PUBLIC ${MEMORY_PROFILER_STUB_LIBRARY_TARGET})
    sober_library_link_library (PUBLIC ${RESOURCE_CONTEXT_HOLDER_LIBRARY_TARGET})
    sober_library_link_library (PUBLIC ${SDL_CONTEXT_HOLDER_LIBRARY_TARGET})
    sober_library_link_library (PUBLIC ${SERIALIZATION_LIBRARY_TARGET})
    sober_library_link_library (PUBLIC SDL3::SDL3)

    sober_library_use_service (PER_VARIANT PRIVATE TestCelerityUIImplementationStrings)
    sober_library_use_service (SHARED PRIVATE Assert)
    sober_library_use_service (SHARED PRIVATE CPUProfiler)
    sober_library_use_service (SHARED PRIVATE JobDispatcher)
    sober_library_use_service (SHARED PRIVATE Log)
    sober_library_use_service (SHARED PRIVATE Memory)
    sober_library_use_service (SHARED PRIVATE MemoryProfiler)
    sober_library_use_service (SHARED PRIVATE RecordCollection)
    sober_library_use_service (SHARED PRIVATE RenderBackend)
    sober_library_use_service (SHARED PRIVATE ResourceProvider)
    sober_library_use_service (SHARED PRIVATE StandardLayoutMapping)
    sober_library_use_service (SHARED PRIVATE TaskExecutor)
    sober_library_use_service (SHARED PRIVATE Testing)
    sober_library_use_service (SHARED PRIVATE VirtualFileSystem)
    sober_library_use_service (SHARED PRIVATE Warehouse)

    # On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
    if (MSVC)
        sober_library_private_compile_options (/EHs)
    endif ()

    sober_test_variant_begin (ImGUI)
        sober_variant_freeze_implementation (TestCelerityUIImplementationStrings ImGUI)
    sober_test_variant_end ()
sober_test_end ()

sober_naming_test_library (CelerityUI TEST_CELERITY_UI_LIBRARY)
sober_naming_library_base_target ("${TEST_CELERITY_UI_LIBRARY}" TEST_CELERITY_UI_LIBRARY_TARGET)

set (RESOURCES_COMPILED_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Resources")
file (MAKE_DIRECTORY "${RESOURCES_COMPILED_DIRECTORY}")

register_resource_usage ("${TEST_CELERITY_UI_LIBRARY_TARGET}"
        "${RESOURCES_COMPILED_DIRECTORY}" "UITest/Compiled" "CoreResources")

register_resource_usage ("${TEST_CELERITY_UI_LIBRARY_TARGET}"
        "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Content" "UITest/Raw" "CoreResources")

register_resource_usage ("${TEST_CELERITY_UI_LIBRARY_TARGET}"
        "${CMAKE_CURRENT_SOURCE_DIR}/Expectation" "UITest/Expectation" "TestResources")

sober_naming_test_collection (CelerityUI COLLECTION_NAME)
add_dependencies (ServiceTests "${COLLECTION_NAME}")
finish_test_setup ()
