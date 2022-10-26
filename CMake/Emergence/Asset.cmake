# Property for saving which asset directories target uses.
define_property (TARGET PROPERTY ASSET_DIRECTORY_MAPPING
        BRIEF_DOCS "List that describes target asset directories."
        FULL_DOCS "\
For each asset directory usage 2 items are added: global path to source directory and relative path to symlink for \
executable that uses this target. Should only be modified through `register_asset_usage` function.")

# Registers that target uses assets from DIRECTORY_GLOBAL_PATH, that should be
# deployed to DEPLOY_RELATIVE_PATH which is relative to runtime output directory.
function (register_asset_usage TARGET DIRECTORY_GLOBAL_PATH DEPLOY_RELATIVE_PATH)
    get_target_property (CURRENT_MAPPING "${TARGET}" ASSET_DIRECTORY_MAPPING)
    if (CURRENT_MAPPING STREQUAL "CURRENT_MAPPING-NOTFOUND")
        set (CURRENT_MAPPING)
    endif ()

    list (APPEND CURRENT_MAPPING "${DIRECTORY_GLOBAL_PATH}")
    list (APPEND CURRENT_MAPPING "${DEPLOY_RELATIVE_PATH}")
    set_target_properties ("${TARGET}" PROPERTIES ASSET_DIRECTORY_MAPPING "${CURRENT_MAPPING}")
endfunction ()

# Private function, should not be used outside of deploy_used_assets.
# Deploy direct (without dependencies) asset usages of given target to given deploy root.
function (private_deploy_direct_assets TARGET DEPLOY_ROOT)
    get_target_property (MAPPING "${TARGET}" ASSET_DIRECTORY_MAPPING)
    if (MAPPING STREQUAL "MAPPING-NOTFOUND")
        return ()
    endif ()

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

# Creates symlinks for all assets used by target and its linked targets in target runtime output directory.
function (deploy_used_assets TARGET)
    get_target_property (DEPLOY_ROOT "${TARGET}" RUNTIME_OUTPUT_DIRECTORY)
    private_deploy_direct_assets ("${TARGET}" "${DEPLOY_ROOT}")
    find_linked_targets_recursively ("${TARGET}" ALL_LINKED_TARGETS)

    foreach (LINKED_TARGET ${ALL_LINKED_TARGETS})
        private_deploy_direct_assets ("${LINKED_TARGET}" "${DEPLOY_ROOT}")
    endforeach ()
endfunction ()
