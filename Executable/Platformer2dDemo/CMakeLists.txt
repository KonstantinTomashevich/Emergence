set (GAME_NAME "Platformer2dDemo")
set (GAME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Games/${GAME_NAME}")
set (GAME_ASSETS "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Development")

add_subdirectory (Game)
add_subdirectory (Logic)
add_subdirectory (Model)
add_subdirectory (Test)

register_asset_usage ("${GAME_NAME}GameLauncher" "${GAME_ASSETS}" "GameAssets")
deploy_used_assets ("${GAME_NAME}GameLauncher")
