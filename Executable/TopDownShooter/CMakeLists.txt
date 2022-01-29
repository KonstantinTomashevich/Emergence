if (DEFINED ENV{URHO3D_DIR})
    file (TO_CMAKE_PATH "$ENV{URHO3D_DIR}" URHO3D_HOME)
    list (APPEND CMAKE_MODULE_PATH "${URHO3D_HOME}/share/CMake/Modules")
else ()
    message (WARNING
            "Urho3D directory is not specified through URHO3D_DIR, therefore TopDownShooter is excluded from project.")
    return ()
endif ()

find_package (Urho3D REQUIRED)
if (NOT URHO3D_FOUND)
    message (WARNING "Urho3D not found, therefore TopDownShooter is excluded from project.")
    return ()
endif ()

add_library (Urho3D SHARED IMPORTED)
set_target_properties (Urho3D PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${URHO3D_INCLUDE_DIRS}"
        IMPORTED_LOCATION "${URHO3D_HOME}/bin/${CMAKE_SHARED_LIBRARY_PREFIX}Urho3D${CMAKE_SHARED_LIBRARY_SUFFIX}"
        IMPORTED_IMPLIB "${URHO3D_LIBRARIES}")

set (GAME_NAME "TopDownShooter")
set (GAME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Games/${GAME_NAME}")

set (GAME_ASSETS "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Converted")
set (RESOURCE_CONFIG_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/resources.cfg")

function (link_resources_for_development OUTPUT_DIRECTORY)
    file (MAKE_DIRECTORY "${OUTPUT_DIRECTORY}")
    file (CREATE_LINK "${URHO3D_HOME}/share/Resources/CoreData"
            "${OUTPUT_DIRECTORY}/Urho3DCoreAssets" SYMBOLIC)
    file (CREATE_LINK "${GAME_ASSETS}" "${OUTPUT_DIRECTORY}/GameAssets" SYMBOLIC)
endfunction ()

add_subdirectory (Editor)
add_subdirectory (Extension)
add_subdirectory (Game)
add_subdirectory (Model)
add_subdirectory (Shared)
add_subdirectory (Test)
