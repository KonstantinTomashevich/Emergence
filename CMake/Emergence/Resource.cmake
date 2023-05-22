# Property for saving which resource directories target uses.
define_property (TARGET PROPERTY RESOURCE_DIRECTORY_MAPPING
        BRIEF_DOCS "List that describes target resource directories."
        FULL_DOCS "\
For each resource directory usage 2 items are added: global path to source directory and relative path to symlink for \
executable that uses this target. Should only be modified through `register_resource_usage` function.")

# Registers that target uses resources from DIRECTORY_GLOBAL_PATH, that should be
# deployed to DEPLOY_RELATIVE_PATH which is relative to runtime output directory.
function (register_resource_usage TARGET DIRECTORY_GLOBAL_PATH DEPLOY_RELATIVE_PATH)
    get_target_property (CURRENT_MAPPING "${TARGET}" RESOURCE_DIRECTORY_MAPPING)
    if (CURRENT_MAPPING STREQUAL "CURRENT_MAPPING-NOTFOUND")
        set (CURRENT_MAPPING)
    endif ()

    list (APPEND CURRENT_MAPPING "${DIRECTORY_GLOBAL_PATH}")
    list (APPEND CURRENT_MAPPING "${DEPLOY_RELATIVE_PATH}")
    set_target_properties ("${TARGET}" PROPERTIES RESOURCE_DIRECTORY_MAPPING "${CURRENT_MAPPING}")
endfunction ()

# Private function, should not be used outside of deploy_used_resources.
# Deploy direct (without dependencies) resource usages of given target to given deploy root.
function (private_deploy_direct_resources TARGET DEPLOY_ROOT)
    get_target_property (MAPPING "${TARGET}" RESOURCE_DIRECTORY_MAPPING)
    if (MAPPING STREQUAL "MAPPING-NOTFOUND")
        return ()
    endif ()

    # Make sure that deploy root exists.
    file (MAKE_DIRECTORY "${DEPLOY_ROOT}")

    list (LENGTH MAPPING MAPPING_LENGTH)
    if (MAPPING_LENGTH GREATER 0)
        math (EXPR LAST_INDEX "${MAPPING_LENGTH} - 2")

        foreach (INDEX RANGE 0 ${LAST_INDEX} 2)
            list (GET MAPPING ${INDEX} SOURCE)
            math (EXPR "NEXT_INDEX" "${INDEX} + 1")
            list (GET MAPPING ${NEXT_INDEX} LOCAL_DESTINATION)
            file (CREATE_LINK "${SOURCE}" "${DEPLOY_ROOT}/${LOCAL_DESTINATION}" SYMBOLIC)
        endforeach ()
    endif ()
endfunction ()

# Creates symlinks for all resources used by target and its linked targets in given deploy root directory
function (deploy_used_resources TARGET DEPLOY_ROOT)
    file (MAKE_DIRECTORY "${DEPLOY_ROOT}")
    private_deploy_direct_resources ("${TARGET}" "${DEPLOY_ROOT}")
    find_linked_targets_recursively ("${TARGET}" ALL_LINKED_TARGETS)

    foreach (LINKED_TARGET ${ALL_LINKED_TARGETS})
        private_deploy_direct_resources ("${LINKED_TARGET}" "${DEPLOY_ROOT}")
    endforeach ()
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
