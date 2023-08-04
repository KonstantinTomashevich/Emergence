define_property (TARGET PROPERTY RESOURCE_DIRECTORY_SOURCES
        BRIEF_DOCS "List of directories that are considered to be sources of resources used by target."
        FULL_DOCS "Forms resource usage mapping along with RESOURCE_DIRECTORY_OUTPUTS and RESOURCE_DIRECTORY_GROUPS.")

define_property (TARGET PROPERTY RESOURCE_DIRECTORY_OUTPUTS
        BRIEF_DOCS "List of virtual file system relative paths for mounting RESOURCE_DIRECTORY_SOURCES."
        FULL_DOCS "Forms resource usage mapping along with RESOURCE_DIRECTORY_SOURCES and RESOURCE_DIRECTORY_GROUPS.")

define_property (TARGET PROPERTY RESOURCE_DIRECTORY_GROUPS
        BRIEF_DOCS "List of mount groups for mounting RESOURCE_DIRECTORY_SOURCES."
        FULL_DOCS "Forms resource usage mapping along with RESOURCE_DIRECTORY_SOURCES and RESOURCE_DIRECTORY_OUTPUTS.")

define_property (TARGET PROPERTY MERGED_RESOURCE_DIRECTORY_SOURCES
        BRIEF_DOCS "Contains result of merging RESOURCE_DIRECTORY_SOURCES from this target and all targets recursively linked to it."
        FULL_DOCS "Forms merged resource usage mapping along with MERGED_RESOURCE_DIRECTORY_OUTPUTS and MERGED_RESOURCE_DIRECTORY_GROUPS.")

define_property (TARGET PROPERTY MERGED_RESOURCE_DIRECTORY_OUTPUTS
        BRIEF_DOCS "Contains result of merging RESOURCE_DIRECTORY_OUTPUTS from this target and all targets recursively linked to it."
        FULL_DOCS "Forms merged resource usage mapping along with MERGED_RESOURCE_DIRECTORY_SOURCES and MERGED_RESOURCE_DIRECTORY_GROUPS.")

define_property (TARGET PROPERTY MERGED_RESOURCE_DIRECTORY_GROUPS
        BRIEF_DOCS "Contains result of merging RESOURCE_DIRECTORY_GROUPS from this target and all targets recursively linked to it."
        FULL_DOCS "Forms merged resource usage mapping along with MERGED_RESOURCE_DIRECTORY_SOURCES and MERGED_RESOURCE_DIRECTORY_OUTPUTS.")

# Common group name used to register resources that are being mounted during startup.
set (CORE_RESOURCE_GROUP "CoreResources")

# Common group name used to register resources that are being used exclusively for testing purposes.
set (TEST_RESOURCE_GROUP "TestResources")

# Informs build system that unit, that is being configured now, uses resources at given source path (directory or
# virtual file system package). In resulting mount configuration this resource will have given virtual output path
# and will be placed in appropriate mount configuration list (one list per group is created).
#
# Arguments:
# - SOURCE: Absolute path to used resource directory or virtual file system package.
# - OUTPUT: Virtual relative path used to mount resource in virtual file system.
# - GROUP: Identifier used to group resource usages into mount lists.
function (register_resource_usage)
    if (NOT DEFINED UNIT_NAME)
        message (FATAL_ERROR "Expected to be called as part of unit configuration routine!")
    endif ()

    cmake_parse_arguments (USAGE "" "SOURCE;OUTPUT;GROUP" "" ${ARGV})
    if (DEFINED USAGE_UNPARSED_ARGUMENTS OR
            NOT DEFINED USAGE_SOURCE OR
            NOT DEFINED USAGE_OUTPUT OR
            NOT DEFINED USAGE_GROUP)
        message (FATAL_ERROR "Incorrect function arguments!")
    endif ()

    get_target_property (SOURCES "${UNIT_NAME}" RESOURCE_DIRECTORY_SOURCES)
    get_target_property (OUTPUTS "${UNIT_NAME}" RESOURCE_DIRECTORY_OUTPUTS)
    get_target_property (GROUPS "${UNIT_NAME}" RESOURCE_DIRECTORY_GROUPS)

    if (SOURCES STREQUAL "SOURCES-NOTFOUND" OR
            OUTPUTS STREQUAL "OUTPUTS-NOTFOUND" OR
            GROUPS STREQUAL "GROUPS-NOTFOUND")

        if (SOURCES STREQUAL "SOURCES-NOTFOUND" AND
                OUTPUTS STREQUAL "OUTPUTS-NOTFOUND" AND
                GROUPS STREQUAL "GROUPS-NOTFOUND")

            set (SOURCES)
            set (OUTPUTS)
            set (GROUPS)

        else ()
            message (FATAL_ERROR "Internal error: encountered not synchronized resource directory zip lists.")
        endif ()

    endif ()

    message (STATUS "    Add resource usage:")
    message (STATUS "        Source: \"${USAGE_SOURCE}\".")
    message (STATUS "        Output: \"${USAGE_OUTPUT}\".")
    message (STATUS "        Group: \"${USAGE_GROUP}\".")

    list (APPEND SOURCES "${USAGE_SOURCE}")
    list (APPEND OUTPUTS "${USAGE_OUTPUT}")
    list (APPEND GROUPS "${USAGE_GROUP}")

    set_target_properties ("${UNIT_NAME}" PROPERTIES
            RESOURCE_DIRECTORY_SOURCES "${SOURCES}"
            RESOURCE_DIRECTORY_OUTPUTS "${OUTPUTS}"
            RESOURCE_DIRECTORY_GROUPS "${GROUPS}")
endfunction ()

# Uses register_resource_usage to register default resource usage scheme for most Emergence units.
# Expects:
# - <SOURCE_DIR>/Resources/Content directory with raw content that doesn't need to be compiled.
# - <SOURCE_DIR>/Resources/Shaders optional directory with BGFX shaders that need to be compiled.
# Creates <BINARY_DIR>/Resources with compiled resources.
function (register_default_resource_usage_scheme)
    if (NOT DEFINED UNIT_NAME)
        message (FATAL_ERROR "Expected to be called as part of unit configuration routine!")
    endif ()

    set (RESOURCES_COMPILED_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Resources")
    file (MAKE_DIRECTORY "${RESOURCES_COMPILED_DIRECTORY}")

    register_bgfx_shaders (
            DX11 SPIRV
            SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shaders"
            OUTPUT "${RESOURCES_COMPILED_DIRECTORY}/Shaders")

    register_resource_usage (
            SOURCE "${RESOURCES_COMPILED_DIRECTORY}"
            OUTPUT "${UNIT_NAME}/Compiled"
            GROUP "${CORE_RESOURCE_GROUP}")

    register_resource_usage (
            SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Content"
            OUTPUT "${UNIT_NAME}/Raw"
            GROUP "${CORE_RESOURCE_GROUP}")
endfunction ()

# Uses register_resource_usage to register default resource usage scheme for most Emergence test units.
# Firstly calls register_default_resource_usage_scheme and then adds <SOURCE_DIR>/Expectation directory
# to test resources group.
function (register_test_default_resource_usage_scheme)
    register_default_resource_usage_scheme ()

    register_resource_usage (
            SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/Expectation"
            OUTPUT "${UNIT_NAME}/Expectation"
            GROUP "TestResources")
endfunction ()

# File-private function, intended only for internal use.
# Merges two resource usage lists. First list variables are expected to be named `${FIRST_PREFIX}_*` and
# second one is `${SECOND_PREFIX}_*`. Result of merging is stored in `${FIRST_PREFIX}_*` variables.
function (private_append_resources FIRST_PREFIX SECOND_PREFIX)
    foreach (USAGE IN ZIP_LISTS ${SECOND_PREFIX}_SOURCES ${SECOND_PREFIX}_OUTPUTS ${SECOND_PREFIX}_GROUPS)
        set (USAGE_SOURCE "${USAGE_0}")
        set (USAGE_OUTPUT "${USAGE_1}")
        set (USAGE_GROUP "${USAGE_2}")

        list (FIND ${FIRST_PREFIX}_OUTPUTS "${USAGE_OUTPUT}" OUTPUT_INDEX)
        if (NOT OUTPUT_INDEX EQUAL -1)
            list (GET ${FIRST_PREFIX}_GROUPS ${OUTPUT_INDEX} SAME_OUTPUT_GROUP)
            if (USAGE_GROUP STREQUAL SAME_OUTPUT_GROUP)
                list (GET ${FIRST_PREFIX}_SOURCES ${OUTPUT_INDEX} SAME_OUTPUT_SOURCE)
                if (USAGE_SOURCE STREQUAL SAME_OUTPUT_SOURCE)
                    continue ()
                else ()
                    message (SEND_ERROR "Found usage with same output \"${USAGE_OUTPUT}\" inside group \"${USAGE_GROUP}\", but different sources: \"${USAGE_SOURCE}\" and \"${SAME_OUTPUT_SOURCE}\".")
                endif ()
            endif ()
        endif ()

        list (APPEND ${FIRST_PREFIX}_SOURCES "${USAGE_SOURCE}")
        list (APPEND ${FIRST_PREFIX}_OUTPUTS "${USAGE_OUTPUT}")
        list (APPEND ${FIRST_PREFIX}_GROUPS "${USAGE_GROUP}")
    endforeach ()

    set (${FIRST_PREFIX}_SOURCES "${${FIRST_PREFIX}_SOURCES}" PARENT_SCOPE)
    set (${FIRST_PREFIX}_OUTPUTS "${${FIRST_PREFIX}_OUTPUTS}" PARENT_SCOPE)
    set (${FIRST_PREFIX}_GROUPS "${${FIRST_PREFIX}_GROUPS}" PARENT_SCOPE)
endfunction ()

# File-private function, intended only for internal use.
# Part of private_request_resource_usage_merged_info routine, adds given target resource usage to merged lists.
function (private_add_direct_resources_to_merged_resources TARGET)
    get_target_property (LOCAL_SOURCES "${TARGET}" RESOURCE_DIRECTORY_SOURCES)
    get_target_property (LOCAL_OUTPUTS "${TARGET}" RESOURCE_DIRECTORY_OUTPUTS)
    get_target_property (LOCAL_GROUPS "${TARGET}" RESOURCE_DIRECTORY_GROUPS)

    if (LOCAL_SOURCES STREQUAL "LOCAL_SOURCES-NOTFOUND" OR
            LOCAL_OUTPUTS STREQUAL "LOCAL_OUTPUTS-NOTFOUND" OR
            LOCAL_GROUPS STREQUAL "LOCAL_GROUPS-NOTFOUND")
        return ()
    endif ()

    private_append_resources ("MERGED" "LOCAL")
    set (MERGED_SOURCES "${MERGED_SOURCES}" PARENT_SCOPE)
    set (MERGED_OUTPUTS "${MERGED_OUTPUTS}" PARENT_SCOPE)
    set (MERGED_GROUPS "${MERGED_GROUPS}" PARENT_SCOPE)
endfunction ()

# File-private function, intended only for internal use.
# Calculates merged resource usage lists for given targets unless it is already calculated.
# Outputs MERGED_SOURCES, MERGED_OUTPUTS and MERGED_GROUPS variables to parent scope.
function (private_request_resource_usage_merged_info TARGET)
    get_target_property (LOCAL_MERGED_SOURCES "${TARGET}" MERGED_RESOURCE_DIRECTORY_SOURCES)
    get_target_property (LOCAL_MERGED_OUTPUTS "${TARGET}" MERGED_RESOURCE_DIRECTORY_OUTPUTS)
    get_target_property (LOCAL_MERGED_GROUPS "${TARGET}" MERGED_RESOURCE_DIRECTORY_GROUPS)

    if (LOCAL_MERGED_SOURCES STREQUAL "LOCAL_MERGED_SOURCES-NOTFOUND" OR
            LOCAL_MERGED_OUTPUTS STREQUAL "LOCAL_MERGED_OUTPUTS-NOTFOUND" OR
            LOCAL_MERGED_GROUPS STREQUAL "LOCAL_MERGED_GROUPS-NOTFOUND")

        set (MERGED_SOURCES)
        set (MERGED_OUTPUTS)
        set (MERGED_GROUPS)

        private_add_direct_resources_to_merged_resources ("${TARGET}")
        find_linked_targets_recursively (TARGET "${TARGET}" OUTPUT ALL_LINKED_TARGETS)

        foreach (LINKED_TARGET ${ALL_LINKED_TARGETS})
            private_add_direct_resources_to_merged_resources ("${LINKED_TARGET}")
        endforeach ()

        set (LOCAL_MERGED_SOURCES "${MERGED_SOURCES}")
        set (LOCAL_MERGED_OUTPUTS "${MERGED_OUTPUTS}")
        set (LOCAL_MERGED_GROUPS "${MERGED_GROUPS}")

        set_target_properties ("${TARGET}" PROPERTIES
                MERGED_RESOURCE_DIRECTORY_SOURCES "${LOCAL_MERGED_SOURCES}"
                MERGED_RESOURCE_DIRECTORY_OUTPUTS "${LOCAL_MERGED_OUTPUTS}"
                MERGED_RESOURCE_DIRECTORY_GROUPS "${LOCAL_MERGED_GROUPS}")
    endif ()

    set (MERGED_SOURCES "${LOCAL_MERGED_SOURCES}" PARENT_SCOPE)
    set (MERGED_OUTPUTS "${LOCAL_MERGED_OUTPUTS}" PARENT_SCOPE)
    set (MERGED_GROUPS "${LOCAL_MERGED_GROUPS}" PARENT_SCOPE)
endfunction ()

# Deploys mount lists for all resources, used by targets from given list and their dependencies, into given deploy
# directory. One mount list is created for every group: MountCoreResources.yaml, MountTestResources.yaml, etc.
# Mount grouping makes it easy to mount and unmount resources at runtime using game-specific logic: for example
# mount and register one resource set for jungle environment and other for desert environment (while using core
# resource group all the time).
function (group_deploy_resource_mount_lists)
    cmake_parse_arguments (GROUP "" "OUTPUT" "TARGETS" ${ARGV})
    if (DEFINED GROUP_UNPARSED_ARGUMENTS OR
            NOT DEFINED GROUP_OUTPUT OR
            NOT DEFINED GROUP_TARGETS)
        message (FATAL_ERROR "Incorrect function arguments!")
    endif ()

    set (GROUP_MERGED_SOURCES)
    set (GROUP_MERGED_OUTPUTS)
    set (GROUP_MERGED_GROUPS)

    foreach (TARGET ${GROUP_TARGETS})
        private_request_resource_usage_merged_info ("${TARGET}")
        private_append_resources ("GROUP_MERGED" "MERGED")
    endforeach ()

    # Make sure that deploy root exists.
    file (MAKE_DIRECTORY "${GROUP_OUTPUT}")
    set (MOUNT_LIST_GROUPS)

    foreach (USAGE IN ZIP_LISTS GROUP_MERGED_SOURCES GROUP_MERGED_OUTPUTS GROUP_MERGED_GROUPS)
        set (USAGE_SOURCE "${USAGE_0}")
        set (USAGE_OUTPUT "${USAGE_1}")
        set (USAGE_GROUP "${USAGE_2}")

        set (VARIABLE_NAME "MOUNT_LIST_${USAGE_GROUP}")
        if (NOT DEFINED ${VARIABLE_NAME})
            set ("${VARIABLE_NAME}")
            string (APPEND "${VARIABLE_NAME}" "# MountConfigurationList\n")
            string (APPEND "${VARIABLE_NAME}" "items:\n")
            list (APPEND MOUNT_LIST_GROUPS "${USAGE_GROUP}")
        endif ()

        if (IS_DIRECTORY "${USAGE_SOURCE}")
            string (APPEND "${VARIABLE_NAME}" "  - source: 0\n")
        else ()
            string (APPEND "${VARIABLE_NAME}" "  - source: 1\n")
        endif ()

        string (APPEND "${VARIABLE_NAME}" "    sourcePath: ${USAGE_SOURCE}\n")
        string (APPEND "${VARIABLE_NAME}" "    targetPath: ${USAGE_OUTPUT}\n")
    endforeach ()

    foreach (GROUP ${MOUNT_LIST_GROUPS})
        set (MOUNT_LIST_VARIABLE "MOUNT_LIST_${GROUP}")
        file (WRITE "${GROUP_OUTPUT}/Mount${GROUP}.yaml" "${${MOUNT_LIST_VARIABLE}}")
    endforeach ()
endfunction ()

# File-private function, intended only for internal use.
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

# File-private function, intended only for internal use.
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

# Registers custom commands for compiling all BGFX shaders from SOURCE directory and storing output in OUTPUT directory.
# Additional options:
# - DX11: Compile to DX11 format.
# - SPIRV: Compile to SPIRV format.
# At least one of the formats should be enabled.
function (register_bgfx_shaders)
    cmake_parse_arguments (SHADER "DX11;SPIRV" "SOURCE;OUTPUT" "" ${ARGV})
    if (DEFINED SHADER_UNPARSED_ARGUMENTS OR
            NOT DEFINED SHADER_SOURCE OR
            NOT DEFINED SHADER_OUTPUT)
        message (FATAL_ERROR "Incorrect function arguments!")
    endif ()

    if (NOT SHADER_DX11 AND NOT SHADER_SPIRV)
        message (SEND_ERROR "No shader targets selected!")
        return ()
    endif ()

    file (GLOB_RECURSE SHADERS "${SHADER_SOURCE}/*.sc")
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

        file (RELATIVE_PATH SHADER_RELATIVE "${SHADER_SOURCE}" "${SHADER}")

        # TODO: We're testing only SPIRV and DX11 now, therefore other shader types are not supported by buildsystem.
        #       Implement support for other shader types.

        if (SHADER_SPIRV)
            string (REPLACE ".sc" ".spirv" SHADER_RELATIVE_BIN_SPIRV "${SHADER_RELATIVE}")
            set (SPIRV_OUTPUT "${SHADER_OUTPUT}/${SHADER_RELATIVE_BIN_SPIRV}")
            private_register_bgfx_spirv_shader (${SHADER} "${TYPE}" "${SPIRV_OUTPUT}")
            list (APPEND OUTPUTS "${SPIRV_OUTPUT}")
        endif ()

        if (SHADER_DX11 AND WIN32)
            string (REPLACE ".sc" ".dx11" SHADER_RELATIVE_BIN_DX11 "${SHADER_RELATIVE}")
            set (DX11_OUTPUT "${SHADER_OUTPUT}/${SHADER_RELATIVE_BIN_DX11}")
            private_register_bgfx_dx11_shader (${SHADER} "${TYPE}" "${SHADER_OUTPUT}/${SHADER_RELATIVE_BIN_DX11}")
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

# Sets up resource cooking targets and distribution packaging targets for given group of targets.
# Arguments:
#    NAME: Name of the packaging configuration to be created.
#    TARGETS: List of targets that need to be packaged. For example, game executable and runtime linked libraries.
#    COOKER: Target executable that is used for cooking resources. Must accept following parameters:
#        --groupName: Name of the mount group to fetch mount list and correctly name final results.
#        --mountListDirectory: Path to directory with all mount lists of given target.
#        --workspace: Path to the workspace directory.
#    COOKING_WORKSPACE: Path to directory where cooking routines may store intermediate files and final results.
#    MOUNT_LIST_DIRECTORY: Path to directory where all mount lists of targets are stored.
#    PACKAGING_OUTPUT: Path to directory when packaging result (game and resources) should be stored.
function (group_setup_resource_cooking_and_packaging)
    cmake_parse_arguments (SETUP "" "NAME;COOKER;COOKING_WORKSPACE;MOUNT_LIST_DIRECTORY;PACKAGING_OUTPUT" "TARGETS" ${ARGV})
    if (DEFINED SETUP_UNPARSED_ARGUMENTS OR
            NOT DEFINED SETUP_NAME OR
            NOT DEFINED SETUP_COOKER OR
            NOT DEFINED SETUP_COOKING_WORKSPACE OR
            NOT DEFINED SETUP_MOUNT_LIST_DIRECTORY OR
            NOT DEFINED SETUP_PACKAGING_OUTPUT OR
            NOT DEFINED SETUP_TARGETS)
        message (FATAL_ERROR "Incorrect function arguments!")
    endif ()

    set (GROUP_MERGED_SOURCES)
    set (GROUP_MERGED_OUTPUTS)
    set (GROUP_MERGED_GROUPS)

    foreach (TARGET ${SETUP_TARGETS})
        private_request_resource_usage_merged_info ("${TARGET}")
        private_append_resources ("GROUP_MERGED" "MERGED")
    endforeach ()
    
    set (UNIQUE_GROUPS)
    foreach (GROUP ${GROUP_MERGED_GROUPS})
        list (FIND UNIQUE_GROUPS "${GROUP}" GROUP_INDEX_IN_LIST)
        if (GROUP_INDEX_IN_LIST EQUAL -1)
            list (APPEND UNIQUE_GROUPS "${GROUP}")
        endif ()
    endforeach ()

    set (COOKING_MAIN_TARGET "${SETUP_NAME}AllResourceCooking")
    add_custom_target (
            "${COOKING_MAIN_TARGET}"
            COMMENT "Cooking all resources for executable \"${SETUP_NAME}\".")

    set (COOKING_MAIN_CLEAN_TARGET "${SETUP_NAME}AllResourceCookingClean")
    add_custom_target (
            "${COOKING_MAIN_CLEAN_TARGET}"
            COMMENT "Cleaning cooking workspace of executable \"${SETUP_NAME}\".")

    set (PACKAGING_MAIN_TARGET "${SETUP_NAME}Packaging")
    add_custom_target (
            "${PACKAGING_MAIN_TARGET}"
            COMMENT "Packaging group \"${SETUP_NAME}\".")
    add_dependencies ("${PACKAGING_MAIN_TARGET}" ${SETUP_TARGETS})

    set (PACKAGING_CLEAN_TARGET "${SETUP_NAME}PackageClean")
    add_custom_target (
            "${PACKAGING_CLEAN_TARGET}"
            COMMENT "Cleaning packaging output of executable \"${SETUP_NAME}\"."
            COMMAND ${CMAKE_COMMAND} -E remove_directory "${SETUP_PACKAGING_OUTPUT}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${SETUP_PACKAGING_OUTPUT}"
            VERBATIM)

    foreach (GROUP ${UNIQUE_GROUPS})
        set (COOKING_TARGET "${SETUP_NAME}${GROUP}Cooking")
        add_custom_target (
                "${COOKING_TARGET}"
                COMMENT "Cooking resource group \"${GROUP}\" for executable \"${SETUP_NAME}\"."
                COMMAND
                $<TARGET_FILE:${SETUP_COOKER}>
                "--groupName" "${GROUP}"
                "--mountListDirectory" "${SETUP_MOUNT_LIST_DIRECTORY}"
                "--workspace" "${SETUP_COOKING_WORKSPACE}/${GROUP}"
                VERBATIM)

        add_dependencies ("${COOKING_TARGET}" "${SETUP_COOKER}")
        add_dependencies ("${COOKING_MAIN_TARGET}" "${COOKING_TARGET}")

        set (COOKING_CLEAN_TARGET "${SETUP_NAME}${GROUP}CookingClean")
        add_custom_target (
                "${COOKING_CLEAN_TARGET}"
                COMMENT "Cleaning cooking workspace for \"${GROUP}\" for executable \"${SETUP_NAME}\"."
                COMMAND ${CMAKE_COMMAND} -E remove_directory "${SETUP_COOKING_WORKSPACE}/${GROUP}"
                VERBATIM)
        add_dependencies ("${COOKING_MAIN_CLEAN_TARGET}" "${COOKING_CLEAN_TARGET}")

        set (PACKAGING_TARGET "${SETUP_NAME}${GROUP}Packaging")
        add_custom_target (
                "${PACKAGING_TARGET}"
                COMMENT "Packaging resource group \"${GROUP}\" for executable \"${SETUP_NAME}\"."
                COMMAND
                ${CMAKE_COMMAND} -E copy_directory
                "${SETUP_COOKING_WORKSPACE}/${GROUP}/FinalResult" "${SETUP_PACKAGING_OUTPUT}"
                VERBATIM)

        add_dependencies ("${PACKAGING_TARGET}" "${PACKAGING_CLEAN_TARGET}" "${COOKING_TARGET}")
        add_dependencies ("${PACKAGING_MAIN_TARGET}" "${PACKAGING_TARGET}")
    endforeach ()
    
    set (RUNTIME_OBJECTS_TO_COPY ${SETUP_TARGETS})
    foreach (TARGET ${SETUP_TARGETS})
        find_linked_shared_libraries (TARGET "${TARGET}" OUTPUT REQUIRED_LIBRARIES)
        list (APPEND RUNTIME_OBJECTS_TO_COPY ${REQUIRED_LIBRARIES})
    endforeach ()
    
    list (REMOVE_DUPLICATES RUNTIME_OBJECTS_TO_COPY)
    foreach (OBJECT_TARGET ${RUNTIME_OBJECTS_TO_COPY})
        set (PACKAGING_TARGET "${SETUP_NAME}${OBJECT_TARGET}Packaging")
        string (REPLACE "::" "_" PACKAGING_TARGET "${PACKAGING_TARGET}")
        get_target_property (TARGET_TYPE "${OBJECT_TARGET}" TYPE)

        if (UNIX AND TARGET_TYPE STREQUAL "SHARED_LIBRARY")
            add_custom_target (
                    ${PACKAGING_TARGET}
                    COMMENT "Packaging shared library \"${OBJECT_TARGET}\" for executable \"${SETUP_NAME}\"."
                    COMMAND
                    ${CMAKE_COMMAND} -E copy $<TARGET_SONAME_FILE:${OBJECT_TARGET}> "${SETUP_PACKAGING_OUTPUT}"
                    VERBATIM)
        else ()
            add_custom_target (
                    ${PACKAGING_TARGET}
                    COMMENT "Packaging shared library \"${OBJECT_TARGET}\" for executable \"${SETUP_NAME}\"."
                    COMMAND
                    ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${OBJECT_TARGET}> "${SETUP_PACKAGING_OUTPUT}"
                    VERBATIM)
        endif ()

        add_dependencies ("${PACKAGING_TARGET}" "${PACKAGING_CLEAN_TARGET}")
        add_dependencies ("${PACKAGING_MAIN_TARGET}" "${PACKAGING_TARGET}")
    endforeach ()
endfunction ()
