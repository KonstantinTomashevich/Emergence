sober_library_begin (TestExportGraphDOT STATIC)
    file (GLOB_RECURSE HEADERS *.hpp)
    file (GLOB_RECURSE SOURCES *.cpp)
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

    sober_naming_variant_target (TestExportGraphCommon Original COMMON_TARGET_NAME)
    sober_library_link_library (PRIVATE ${COMMON_TARGET_NAME})

    sober_library_use_service (Testing PRIVATE)
    sober_library_use_service (ExportGraph PRIVATE)

    sober_variant_begin (Original)
        sober_variant_freeze_implementation (ExportGraph DOT)
    sober_variant_end ()
sober_library_end ()

sober_naming_variant_target (TestExportGraphDOT Original VARIANT_TARGET_NAME)
sober_naming_test_runner ("${VARIANT_TARGET_NAME}" RUNNER_TARGET_NAME)
sober_naming_test_runner_ctest_name ("${VARIANT_TARGET_NAME}" RUNNER_CTEST_NAME)

add_executable ("${RUNNER_TARGET_NAME}" "${SOBER_TEST_RUNNER_STUB_SOURCE}")
target_link_libraries ("${RUNNER_TARGET_NAME}" PRIVATE "${VARIANT_TARGET_NAME}")

add_test (NAME "${RUNNER_CTEST_NAME}" COMMAND "${RUNNER_TARGET_NAME}")
add_dependencies (ServiceTests "${RUNNER_TARGET_NAME}")
