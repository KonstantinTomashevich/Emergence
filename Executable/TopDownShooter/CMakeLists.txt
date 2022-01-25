find_package (OGRE REQUIRED COMPONENTS Bites RTShaderSystem CONFIG)

set (GAME_NAME "TopDownShooter")
set (GAME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Games/${GAME_NAME}")
set (GAME_RENDER_SYSTEM Direct3D11)

function (use_ogre_plugins OUTPUT_DIRECTORY)
    set (CONFIG_LINES ${ARGN})
    list (TRANSFORM CONFIG_LINES PREPEND "Plugin=")
    list (TRANSFORM CONFIG_LINES APPEND "\n")
    list (JOIN CONFIG_LINES "" CONFIG_CONTENT)
    file (WRITE "${OUTPUT_DIRECTORY}/plugins.cfg" ${CONFIG_CONTENT})

    foreach (PLUGIN ${ARGN})
        set (LIBRARY_NAME "${CMAKE_SHARED_LIBRARY_PREFIX}${PLUGIN}${CMAKE_SHARED_LIBRARY_SUFFIX}")
        file (COPY_FILE
                "${OGRE_PLUGIN_DIR}/${LIBRARY_NAME}" "${OUTPUT_DIRECTORY}/${LIBRARY_NAME}" ONLY_IF_DIFFERENT)
    endforeach ()
endfunction ()

set (GAME_ASSETS "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Converted")
set (RESOURCE_CONFIG_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/resources.cfg")

function (link_resources_for_development OUTPUT_DIRECTORY)
    configure_file ("${RESOURCE_CONFIG_TEMPLATE}" "${OUTPUT_DIRECTORY}/resources.cfg")
endfunction ()

add_subdirectory (Editor)
add_subdirectory (Extension)
add_subdirectory (Game)
add_subdirectory (Model)
add_subdirectory (Shared)
add_subdirectory (Test)
