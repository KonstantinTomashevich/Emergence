set (GAME_NAME "Platformer2dDemo")
set (GAME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Games/${GAME_NAME}")
set (GAME_RESOURCES_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/Resources")
set (GAME_RESOURCES_COMPILED "${CMAKE_CURRENT_BINARY_DIR}/Resources")

file (GLOB_RECURSE GAME_SHADERS "${GAME_RESOURCES_SOURCE}/*.sc")
add_library ("${GAME_NAME}Shaders" INTERFACE ${GAME_SHADERS})

add_subdirectory (Game)
add_subdirectory (Logic)
add_subdirectory (Model)

file (MAKE_DIRECTORY "${GAME_RESOURCES_COMPILED}")
register_bgfx_shaders ("${GAME_RESOURCES_SOURCE}/Shaders" "${GAME_RESOURCES_COMPILED}/Shaders")

register_resource_usage ("${GAME_NAME}GameLauncher"
        "${GAME_RESOURCES_COMPILED}" "Game/Compiled" "CoreResources")

register_resource_usage ("${GAME_NAME}GameLauncher"
        "${GAME_RESOURCES_SOURCE}/Content" "Game/Raw" "CoreResources")

deploy_resource_mount_lists ("${GAME_NAME}GameLauncher" "${GAME_OUTPUT_DIRECTORY}/Game")
