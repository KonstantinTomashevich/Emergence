# Property for saving which resource directories target uses.
define_property (TARGET PROPERTY RESOURCE_DIRECTORY_MAPPING
        BRIEF_DOCS "List that describes target resource directories."
        FULL_DOCS "\
For each resource usage 3 items are added: global path to source, virtual path for mounting and group name for \
grouping mount lists. Should only be modified through `register_resource_usage` function.")

# Registers that given target uses resources at given real path (directory or virtual file system package).
# In resulting mount configuration this resource will have given virtual target path and will be placed in
# appropriate mount configuration list (one list per group is created).
function (register_resource_usage TARGET REAL_PATH VIRTUAL_PATH GROUP)
    get_target_property (CURRENT_MAPPING "${TARGET}" RESOURCE_DIRECTORY_MAPPING)
    if (CURRENT_MAPPING STREQUAL "CURRENT_MAPPING-NOTFOUND")
        set (CURRENT_MAPPING)
    endif ()

    list (APPEND CURRENT_MAPPING "${REAL_PATH}")
    list (APPEND CURRENT_MAPPING "${VIRTUAL_PATH}")
    list (APPEND CURRENT_MAPPING "${GROUP}")
    set_target_properties ("${TARGET}" PROPERTIES RESOURCE_DIRECTORY_MAPPING "${CURRENT_MAPPING}")
endfunction ()

# File-private function that extracts given target RESOURCE_DIRECTORY_MAPPING
# and inserts it into MERGED_MAPPING in the parent scope.
function (private_add_direct_resources_to_merged_mapping TARGET)
    get_target_property (MAPPING "${TARGET}" RESOURCE_DIRECTORY_MAPPING)
    if (MAPPING STREQUAL "MAPPING-NOTFOUND")
        return ()
    endif ()

    set (LOCAL_MERGED_MAPPING "${MERGED_MAPPING}")
    list (LENGTH MAPPING MAPPING_LENGTH)

    if (MAPPING_LENGTH GREATER 0)
        math (EXPR LAST_INDEX "${MAPPING_LENGTH} - 3")

        foreach (INDEX RANGE 0 ${LAST_INDEX} 3)
            list (GET MAPPING ${INDEX} REAL_PATH)
            math (EXPR "VIRTUAL_PATH_INDEX" "${INDEX} + 1")
            list (GET MAPPING ${VIRTUAL_PATH_INDEX} VIRTUAL_PATH)
            math (EXPR "GROUP_INDEX" "${INDEX} + 2")
            list (GET MAPPING ${GROUP_INDEX} GROUP)

            list (APPEND LOCAL_MERGED_MAPPING "${REAL_PATH}")
            list (APPEND LOCAL_MERGED_MAPPING "${VIRTUAL_PATH}")
            list (APPEND LOCAL_MERGED_MAPPING "${GROUP}")
        endforeach ()
    endif ()

    set (MERGED_MAPPING "${LOCAL_MERGED_MAPPING}" PARENT_SCOPE)
endfunction ()

# Deploys mount lists for all resources, used by given target and its dependencies, into given deploy directory.
# One mount list is created for every group: MountCoreResources.yaml, MountTestResources.yaml, etc.
# Mount grouping makes it easy to mount and unmount resources at runtime using game-specific logic: for example
# mount and register one resource set for jungle environment and other for desert environment (while using core
# resource group all the time).
function (deploy_resource_mount_lists TARGET DEPLOY_DIRECTORY)
    set (MERGED_MAPPING)
    private_add_direct_resources_to_merged_mapping ("${TARGET}")
    sober_find_linked_targets_recursively ("${TARGET}" ALL_LINKED_TARGETS)

    foreach (LINKED_TARGET ${ALL_LINKED_TARGETS})
        private_add_direct_resources_to_merged_mapping ("${LINKED_TARGET}")
    endforeach ()

    # Make sure that deploy root exists.
    file (MAKE_DIRECTORY "${DEPLOY_DIRECTORY}")
    set (MOUNT_LISTS)
    set (MOUNT_LIST_NAMES)
    list (LENGTH MERGED_MAPPING MERGED_MAPPING_LENGTH)

    if (MERGED_MAPPING_LENGTH GREATER 0)
        math (EXPR LAST_INDEX "${MERGED_MAPPING_LENGTH} - 3")

        foreach (INDEX RANGE 0 ${LAST_INDEX} 3)
            list (GET MERGED_MAPPING ${INDEX} REAL_PATH)
            math (EXPR "VIRTUAL_PATH_INDEX" "${INDEX} + 1")
            list (GET MERGED_MAPPING ${VIRTUAL_PATH_INDEX} VIRTUAL_PATH)
            math (EXPR "GROUP_INDEX" "${INDEX} + 2")
            list (GET MERGED_MAPPING ${GROUP_INDEX} GROUP)

            set (VARIABLE_NAME "MOUNT_LIST_${GROUP}")
            list (FIND MOUNT_LISTS "${VARIABLE_NAME}" VARIABLE_INDEX)

            if (VARIABLE_INDEX EQUAL -1)
                set ("${VARIABLE_NAME}")
                string (APPEND "${VARIABLE_NAME}" "# MountConfigurationList\n")
                string (APPEND "${VARIABLE_NAME}" "items:\n")

                list (APPEND MOUNT_LISTS "${VARIABLE_NAME}")
                list (APPEND MOUNT_LIST_NAMES "${GROUP}")
            endif ()

            if (IS_DIRECTORY "${REAL_PATH}")
                string (APPEND "${VARIABLE_NAME}" "  - source: 0\n")
            else ()
                string (APPEND "${VARIABLE_NAME}" "  - source: 1\n")
            endif ()

            string (APPEND "${VARIABLE_NAME}" "    sourcePath: ${REAL_PATH}\n")
            string (APPEND "${VARIABLE_NAME}" "    targetPath: ${VIRTUAL_PATH}\n")
        endforeach ()
    endif ()

    list (LENGTH MOUNT_LISTS MOUNT_LISTS_LENGTH)
    if (MOUNT_LISTS_LENGTH GREATER 0)
        math (EXPR LAST_INDEX "${MOUNT_LISTS_LENGTH} - 1")

        foreach (INDEX RANGE 0 ${LAST_INDEX})
            list (GET MOUNT_LISTS ${INDEX} MOUNT_LIST_VARIABLE)
            list (GET MOUNT_LIST_NAMES ${INDEX} MOUNT_LIST_NAME)
            file (WRITE "${DEPLOY_DIRECTORY}/Mount${MOUNT_LIST_NAME}.yaml" "${${MOUNT_LIST_VARIABLE}}")
        endforeach ()
    endif ()
endfunction ()

# Private function, should not be used outside of register_bgfx_shaders.
# Registers custom command for compiling BGFX shader to SPIRV format.
function (private_register_bgfx_spirv_shader SHADER_INPUT SHADER_TYPE SHADER_OUTPUT)
    get_filename_component (SHADER_INPUT_DIRECTORY "${SHADER_INPUT}" DIRECTORY)
    get_filename_component (SHADER_OUTPUT_DIRECTORY "${SHADER_OUTPUT}" DIRECTORY)
    file (MAKE_DIRECTORY "${SHADER_OUTPUT_DIRECTORY}")

    shaderc_parse (COMPILE_ARGUMENTS
            FILE "${SHADER_INPUT}" "${SHADER_TYPE}"
            # TODO: Support for shader includes.
            INCLUDES "${BGFX_DIR}/src"
            LINUX PROFILE spirv
            OUTPUT "${SHADER_OUTPUT}")

    list (APPEND SHADER_COMPILATION_COMMANDS COMMAND "$<TARGET_FILE:shaderc>" ${COMPILE_ARGUMENTS})
    set (SHADER_COMPILATION_COMMANDS "${SHADER_COMPILATION_COMMANDS}" PARENT_SCOPE)
endfunction ()

# Private function, should not be used outside of register_bgfx_shaders.
# Registers custom command for compiling BGFX shader to DX11 format.
function (private_register_bgfx_dx11_shader SHADER_INPUT SHADER_TYPE SHADER_OUTPUT)
    get_filename_component (SHADER_OUTPUT_DIRECTORY "${SHADER_OUTPUT}" DIRECTORY)
    file (MAKE_DIRECTORY "${SHADER_OUTPUT_DIRECTORY}")

    if ("${SHADER_TYPE}" STREQUAL "FRAGMENT")
        set (D3D_PREFIX "ps")
    elseif ("${TYPE}" STREQUAL "VERTEX")
        set (D3D_PREFIX "vs")
    endif ()

    shaderc_parse (COMPILE_ARGUMENTS
            FILE "${SHADER_INPUT}" "${SHADER_TYPE}"
            # TODO: Support for shader includes.
            INCLUDES "${BGFX_DIR}/src"
            WINDOWS PROFILE ${D3D_PREFIX}_5_0 O 3
            OUTPUT "${SHADER_OUTPUT}")

    list (APPEND SHADER_COMPILATION_COMMANDS COMMAND "$<TARGET_FILE:shaderc>" ${COMPILE_ARGUMENTS})
    set (SHADER_COMPILATION_COMMANDS "${SHADER_COMPILATION_COMMANDS}" PARENT_SCOPE)
endfunction ()

# Registers custom commands for compiling all BGFX shaders from source directory.
# Compiled shaders will be saved to given binary directory.
function (register_bgfx_shaders SOURCE_DIRECTORY BINARY_DIRECTORY)
    file (GLOB_RECURSE SHADERS "${SOURCE_DIRECTORY}/*.sc")
    foreach (SHADER ${SHADERS})
        if ("${SHADER}" MATCHES ".*def\\.sc")
            continue ()
        endif ()

        if ("${SHADER}" MATCHES ".*\\.fragment\\.sc")
            set (TYPE "FRAGMENT")
        elseif ("${SHADER}" MATCHES ".*\\.vertex\\.sc")
            set (TYPE "VERTEX")
        else ()
            message (WARNING "Shader ${SHADER_INPUT} is not getting compiled: unknown type.")
            continue ()
        endif ()

        file (RELATIVE_PATH SHADER_RELATIVE "${SOURCE_DIRECTORY}" "${SHADER}")

        # TODO: We're testing only SPIRV and DX11 now, therefore other shader types are not supported by buildsystem.
        #       Implement support for other shader types.

        string (REPLACE ".sc" ".spirv" SHADER_RELATIVE_BIN_SPIRV "${SHADER_RELATIVE}")
        set (SPIRV_OUTPUT "${BINARY_DIRECTORY}/${SHADER_RELATIVE_BIN_SPIRV}")
        private_register_bgfx_spirv_shader (${SHADER} "${TYPE}" "${SPIRV_OUTPUT}")
        list (APPEND OUTPUTS "${SPIRV_OUTPUT}")

        if (WIN32)
            string (REPLACE ".sc" ".dx11" SHADER_RELATIVE_BIN_DX11 "${SHADER_RELATIVE}")
            set (DX11_OUTPUT "${BINARY_DIRECTORY}/${SHADER_RELATIVE_BIN_DX11}")
            private_register_bgfx_dx11_shader (${SHADER} "${TYPE}" "${BINARY_DIRECTORY}/${SHADER_RELATIVE_BIN_DX11}")
            list (APPEND OUTPUTS "${DX11_OUTPUT}")
        endif ()

        get_filename_component (SHADER_DIRECTORY "${SHADER}" DIRECTORY)
        add_custom_command (
                MAIN_DEPENDENCY "${SHADER}"
                DEPENDS "${SHADER_DIRECTORY}/varying.def.sc"
                OUTPUT ${OUTPUTS}
                ${SHADER_COMPILATION_COMMANDS}
                COMMENT "Compiling ${SHADER}...")

        unset (OUTPUTS)
        unset (SHADER_COMPILATION_COMMANDS)
    endforeach ()
endfunction ()
