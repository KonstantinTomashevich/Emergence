sober_test_begin (CelerityRender2d)
    file (GLOB_RECURSE SOURCES *.cpp)
    file (GLOB_RECURSE HEADERS *.hpp)
    file (GLOB_RECURSE SHADERS *.sc)
    list (APPEND SOURCES ${HEADERS} ${SHADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

    sober_naming_variant_target (CelerityAsset Original CELERITY_ASSET_LIBRARY_TARGET)
    sober_naming_variant_target (CelerityTransform Original CELERITY_TRANSFORM_LIBRARY_TARGET)
    sober_naming_variant_target (MemoryProfilerStub Original MEMORY_PROFILER_STUB_LIBRARY_TARGET)

    sober_library_link_library (PUBLIC ${CELERITY_ASSET_LIBRARY_TARGET})
    sober_library_link_library (PUBLIC ${CELERITY_TRANSFORM_LIBRARY_TARGET})
    sober_library_link_library (PUBLIC ${MEMORY_PROFILER_STUB_LIBRARY_TARGET})
    sober_library_link_library (PUBLIC SDL2)
    sober_library_link_library (PUBLIC SDL2main)

    sober_library_use_service (SHARED PRIVATE Assert)
    sober_library_use_service (SHARED PRIVATE JobDispatcher)
    sober_library_use_service (SHARED PRIVATE Log)
    sober_library_use_service (SHARED PRIVATE Memory)
    sober_library_use_service (SHARED PRIVATE MemoryProfiler)
    sober_library_use_service (SHARED PRIVATE RecordCollection)
    sober_library_use_service (SHARED PRIVATE StandardLayoutMapping)
    sober_library_use_service (SHARED PRIVATE TaskExecutor)
    sober_library_use_service (SHARED PRIVATE Testing)
    sober_library_use_service (SHARED PRIVATE Warehouse)

    # On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
    if (MSVC)
        sober_library_private_compile_options (/EHs)
    endif ()

    sober_test_variant_begin (BGFX)
    sober_test_variant_end ()
sober_test_end ()

set (ASSETS_COMPILED_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Assets")
file (MAKE_DIRECTORY "${ASSETS_COMPILED_DIRECTORY}")
file (CREATE_LINK "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Expectation" "${ASSETS_COMPILED_DIRECTORY}/Expectation" SYMBOLIC)
file (CREATE_LINK "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Materials" "${ASSETS_COMPILED_DIRECTORY}/Materials" SYMBOLIC)
file (CREATE_LINK "${CMAKE_CURRENT_SOURCE_DIR}/Assets/MaterialInstances"
        "${ASSETS_COMPILED_DIRECTORY}/MaterialInstances" SYMBOLIC)
file (CREATE_LINK "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Textures" "${ASSETS_COMPILED_DIRECTORY}/Textures" SYMBOLIC)
register_bgfx_shaders ("${CMAKE_CURRENT_SOURCE_DIR}/Assets/Shaders" "${ASSETS_COMPILED_DIRECTORY}/Shaders")

sober_naming_test_library (CelerityRender2d CELERITY_RENDER_2D_TEST_LIBRARY)
sober_naming_library_base_target ("${CELERITY_RENDER_2D_TEST_LIBRARY}" CELERITY_RENDER_2D_TEST_LIBRARY_TARGET)
register_asset_usage ("${CELERITY_RENDER_2D_TEST_LIBRARY_TARGET}" "${ASSETS_COMPILED_DIRECTORY}" "Render2dAssets")

if (EMERGENCE_INCLUDE_GPU_DEPENDANT_TESTS)
    sober_naming_test_collection (CelerityRender2d COLLECTION_NAME)
    add_dependencies (ServiceTests "${COLLECTION_NAME}")
endif ()

finish_test_setup ()
