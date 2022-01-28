if (DEFINED ENV{URHO3D_DIR})
    file (TO_CMAKE_PATH "$ENV{URHO3D_DIR}" CONVERTED_PATH)
    list (APPEND CMAKE_MODULE_PATH "${CONVERTED_PATH}/share/CMake/Modules")
endif ()

find_package (Urho3D REQUIRED)

if (NOT URHO3D_FOUND)
    message (WARNING "Urho3D not found, therefore TopDownShooter is excluded from project.")
    return ()
endif ()


set (GAME_NAME "TopDownShooter")
set (GAME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Games/${GAME_NAME}")

set (GAME_ASSETS "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Converted")
set (RESOURCE_CONFIG_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/resources.cfg")

function (link_resources_for_development OUTPUT_DIRECTORY)
    # TODO: Symlinks to Urho3D core data and our assets.
endfunction ()

add_subdirectory (Editor)
add_subdirectory (Extension)
add_subdirectory (Game)
add_subdirectory (Model)
add_subdirectory (Shared)
add_subdirectory (Test)
