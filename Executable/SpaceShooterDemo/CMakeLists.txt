if (NOT TARGET Urho3D)
    message (WARNING "Urho3D not found, therefore SpaceShooterDemo is excluded from project.")
    return ()
endif ()

set (GAME_NAME "SpaceShooterDemo")
set (GAME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Games/${GAME_NAME}")
set (GAME_RENDER_SYSTEM Direct3D11)
set (GAME_RESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Converted")

# We need to mark game executables with WIN32 on Windows.
set (GAME_EXECUTABLE_FLAG "")

if (WIN32)
    set (GAME_EXECUTABLE_FLAG "WIN32")
endif ()

add_custom_target (SpaceShooterDemoConvertIntermediateResources
        powershell ".\\Scripts\\ConvertResources.ps1" "${URHO3D_HOME}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory (Game)
add_subdirectory (Model)
add_subdirectory (Shared)

register_resource_usage ("${GAME_NAME}GameLauncher" "${URHO3D_HOME}/share/Resources/CoreData" "Urho3DCoreResources")
register_resource_usage ("${GAME_NAME}GameLauncher" "${GAME_RESOURCES}" "GameResources")
deploy_used_resources ("${GAME_NAME}GameLauncher" "${GAME_OUTPUT_DIRECTORY}/Game/Resources")
