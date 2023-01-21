set (GAME_NAME "Platformer2dDemo")
set (GAME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Games/${GAME_NAME}")
set (GAME_RESOURCES_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Development")
set (GAME_RESOURCES_TARGET "${CMAKE_CURRENT_BINARY_DIR}/Resources/Development")

file (GLOB_RECURSE GAME_SHADERS "${GAME_RESOURCES_SOURCE}/*.sc")
add_library ("${GAME_NAME}Shaders" INTERFACE ${GAME_SHADERS})

add_subdirectory (Game)
add_subdirectory (Logic)
add_subdirectory (Model)
add_subdirectory (Test)

file (MAKE_DIRECTORY "${GAME_RESOURCES_TARGET}")
file (CREATE_LINK "${GAME_RESOURCES_SOURCE}/Fonts" "${GAME_RESOURCES_TARGET}/Fonts" SYMBOLIC)
file (CREATE_LINK "${GAME_RESOURCES_SOURCE}/Materials" "${GAME_RESOURCES_TARGET}/Materials" SYMBOLIC)
file (CREATE_LINK "${GAME_RESOURCES_SOURCE}/MaterialInstances" "${GAME_RESOURCES_TARGET}/MaterialInstances" SYMBOLIC)
file (CREATE_LINK "${GAME_RESOURCES_SOURCE}/Textures" "${GAME_RESOURCES_TARGET}/Textures" SYMBOLIC)

register_bgfx_shaders ("${GAME_RESOURCES_SOURCE}/Shaders" "${GAME_RESOURCES_TARGET}/Shaders")
register_resource_usage ("${GAME_NAME}GameLauncher" "${GAME_RESOURCES_TARGET}" "GameResources")
deploy_used_resources ("${GAME_NAME}GameLauncher")
