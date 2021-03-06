sober_test_begin (CelerityPhysics)
    file (GLOB_RECURSE SOURCES *.cpp)
    file (GLOB_RECURSE HEADERS *.hpp)
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

    sober_naming_variant_target (CelerityTransform Original CELERITY_TRANSFORM_LIBRARY_TARGET)
    sober_naming_variant_target (MemoryProfilerStub Original MEMORY_PROFILER_STUB_LIBRARY_TARGET)

    sober_library_link_library (PUBLIC ${CELERITY_TRANSFORM_LIBRARY_TARGET})
    sober_library_link_library (PUBLIC ${MEMORY_PROFILER_STUB_LIBRARY_TARGET})

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

    sober_test_variant_begin (PhysX)
    sober_test_variant_end ()
sober_test_end ()

sober_naming_test_collection (CelerityPhysics COLLECTION_NAME)
add_dependencies (ServiceTests "${COLLECTION_NAME}")

get_property (TEST_TARGETS DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY BUILDSYSTEM_TARGETS)
foreach (TARGET IN LISTS TEST_TARGETS)
    get_target_property (TARGET_TYPE "${TARGET}" TYPE)
    if (TARGET_TYPE STREQUAL "EXECUTABLE")
        copy_required_shared_libraries (${TARGET})
    endif ()
endforeach ()
