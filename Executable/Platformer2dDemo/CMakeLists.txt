set (GAME_NAME "Platformer2dDemo")
set (GAME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Games/${GAME_NAME}")
set (GAME_ASSETS "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Development")

function (link_resources_for_development OUTPUT_DIRECTORY)
    file (MAKE_DIRECTORY "${OUTPUT_DIRECTORY}")
    file (CREATE_LINK "${GAME_ASSETS}" "${OUTPUT_DIRECTORY}/GameAssets" SYMBOLIC)
endfunction ()

add_subdirectory (Game)
add_subdirectory (Logic)
add_subdirectory (Model)
add_subdirectory (Test)
