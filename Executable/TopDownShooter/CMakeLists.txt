if (NOT TARGET Urho3D)
    message (WARNING "Urho3D not found, therefore TopDownShooter is excluded from project.")
    return ()
endif ()

set (GAME_NAME "TopDownShooter")
set (GAME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Games/${GAME_NAME}")
set (GAME_RENDER_SYSTEM Direct3D11)
set (GAME_ASSETS "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Converted")

function (link_resources_for_development OUTPUT_DIRECTORY)
    file (MAKE_DIRECTORY "${OUTPUT_DIRECTORY}")
    file (CREATE_LINK "${URHO3D_HOME}/share/Resources/CoreData" "${OUTPUT_DIRECTORY}/Urho3DCoreAssets" SYMBOLIC)
    file (CREATE_LINK "${GAME_ASSETS}" "${OUTPUT_DIRECTORY}/GameAssets" SYMBOLIC)
endfunction ()

set (URHO3D_EXECUTABLE_TYPE "")
if (WIN32)
    set (URHO3D_EXECUTABLE_TYPE "WIN32")
endif ()

add_custom_target (TopDownShooterConvertIntermediateAssets
        powershell ".\\Scripts\\ConvertAssets.ps1" "${URHO3D_HOME}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory (Editor)
add_subdirectory (Extension)
add_subdirectory (Game)
add_subdirectory (Model)
add_subdirectory (Shared)
add_subdirectory (Test)
