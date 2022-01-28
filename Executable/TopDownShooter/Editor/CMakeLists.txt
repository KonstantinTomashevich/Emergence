sober_library_begin ("${GAME_NAME}Editor" STATIC)
    file (GLOB_RECURSE HEADERS *.hpp)
    file (GLOB_RECURSE SOURCES *.cpp)
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

    sober_naming_variant_target ("${GAME_NAME}Shared" Original SHARED_TARGET)
    sober_library_link_library (PUBLIC ${SHARED_TARGET})

    sober_library_use_service (PER_VARIANT PRIVATE Log)
    sober_library_use_service (PER_VARIANT PRIVATE StandardLayoutMapping)

    sober_variant_begin (Original)
    sober_variant_end ()
sober_library_end ()

add_executable ("${GAME_NAME}EditorLauncher" "${SOBER_TEST_RUNNER_STUB_SOURCE}")
sober_naming_variant_target ("${GAME_NAME}Editor" Original EDITOR_TARGET)
target_link_libraries ("${GAME_NAME}EditorLauncher" PUBLIC "${EDITOR_TARGET}")

set_target_properties ("${GAME_NAME}EditorLauncher" PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${GAME_OUTPUT_DIRECTORY}/Editor")

copy_required_shared_libraries ("${GAME_NAME}EditorLauncher")
link_resources_for_development ("${GAME_OUTPUT_DIRECTORY}/Editor")
