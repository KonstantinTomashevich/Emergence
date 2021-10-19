sober_library_begin ("${GAME_NAME}Editor" STATIC)
    file (GLOB_RECURSE HEADERS *.hpp)
    file (GLOB_RECURSE SOURCES *.cpp)
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

    sober_library_link_library (PUBLIC OgreBites)
    sober_library_link_library (PUBLIC OgreRTShaderSystem)

    sober_naming_variant_target ("${GAME_NAME}Model" Original MODEL_TARGET)
    sober_library_link_library (PUBLIC ${MODEL_TARGET})

    sober_naming_variant_target ("${GAME_NAME}Shared" Original SHARED_TARGET)
    sober_library_link_library (PUBLIC ${SHARED_TARGET})

    sober_variant_begin (Original)
    sober_variant_end ()
sober_library_end ()

add_executable ("${GAME_NAME}EditorLauncher" "${SOBER_TEST_RUNNER_STUB_SOURCE}")
sober_naming_variant_target ("${GAME_NAME}Editor" Original EDITOR_TARGET)
target_link_libraries ("${GAME_NAME}EditorLauncher" PUBLIC "${EDITOR_TARGET}")

set_target_properties ("${GAME_NAME}EditorLauncher" PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${GAME_OUTPUT_DIRECTORY}/Editor")
copy_shared_libraries ("${GAME_NAME}EditorLauncher")
use_ogre_plugins ("${GAME_OUTPUT_DIRECTORY}/Editor" RenderSystem_Direct3D11)

set (RESOURCES_CONFIG "[General]\nFileSystem=../Assets\n")
file (WRITE "${GAME_OUTPUT_DIRECTORY}/Editor/resources.cfg" "${RESOURCES_CONFIG}")
