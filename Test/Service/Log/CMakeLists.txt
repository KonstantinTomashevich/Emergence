set (IMPLEMENTATIONS_TO_TEST SPDLog)

sober_library_begin (TestLogShared STATIC)
    file (GLOB_RECURSE SOURCES "Shared/*.cpp")
    file (GLOB_RECURSE HEADERS "Shared/*.hpp")
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Shared")

    sober_library_use_service (EXPECTED PUBLIC Log)

    sober_variant_begin (Original)
    sober_variant_end ()
sober_library_end ()

sober_naming_variant_target (TestLogShared Original LOG_TEST_SHARED_LIBRARY)

sober_library_begin (TestLogApp STATIC)
    file (GLOB_RECURSE SOURCES "App/*.cpp")
    file (GLOB_RECURSE HEADERS "App/*.hpp")
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/App")
    sober_library_link_library (PRIVATE ${LOG_TEST_SHARED_LIBRARY})

    sober_library_use_service (PER_VARIANT PRIVATE Log)
    sober_library_use_service (SHARED PRIVATE Memory)
    sober_library_use_service (SHARED PRIVATE MemoryProfiler)

    foreach (IMPLEMENTATION ${IMPLEMENTATIONS_TO_TEST})
        sober_variant_begin (${IMPLEMENTATION})
            sober_variant_freeze_implementation (Log ${IMPLEMENTATION})
        sober_variant_end ()
    endforeach ()
sober_library_end ()

add_executable (TestLogDriver "${CMAKE_CURRENT_SOURCE_DIR}/Driver/Log/Test/Main.cpp")
target_link_libraries (TestLogDriver PRIVATE ${LOG_TEST_SHARED_LIBRARY})
add_dependencies (ServiceTests TestLogDriver)

foreach (IMPLEMENTATION ${IMPLEMENTATIONS_TO_TEST})
    set (RUNNER_TARGET "TestLogApp${IMPLEMENTATION}Runner")
    add_executable ("${RUNNER_TARGET}" ${SOBER_TEST_RUNNER_STUB_SOURCE})
    sober_naming_variant_target (TestLogApp ${IMPLEMENTATION} IMPLEMENTATION_LIBRARY)
    target_link_libraries ("${RUNNER_TARGET}" PRIVATE ${IMPLEMENTATION_LIBRARY})

    add_dependencies (ServiceTests "${RUNNER_TARGET}")
    add_test (NAME "${RUNNER_TARGET}" COMMAND TestLogDriver $<TARGET_FILE:${RUNNER_TARGET}>)
endforeach ()
