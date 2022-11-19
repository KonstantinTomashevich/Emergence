# Adds custom command that copies shared libraries, requested by given target.
function (copy_required_shared_libraries TARGET)
    add_custom_command (
            TARGET "${TARGET}" POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:${TARGET}> $<TARGET_FILE_DIR:${TARGET}>
            COMMAND_EXPAND_LISTS)
endfunction ()

# Writes given content to given file, unless it already has equal content.
# Useful for code generation: allows to avoid unnecessary recompilation.
function (write_if_not_equal FILE CONTENT)
    set (CURRENT_CONTENT)

    if (EXISTS "${FILE}")
        file (READ "${FILE}" CURRENT_CONTENT)
    endif ()

    if (NOT CONTENT STREQUAL CURRENT_CONTENT)
        file (WRITE "${FILE}" "${CONTENT}")
    endif ()
endfunction ()

# Executes CMake-driven code generation by including all "*.generated.*.cmake" files.
function (call_code_generation ROOT_DIRECTORY)
    file (GLOB_RECURSE GENERATORS "${ROOT_DIRECTORY}/*.generated.*.cmake")
    foreach (GENERATOR IN LISTS GENERATORS)
        set (TARGET_FILE "${GENERATOR}")
        cmake_path (REMOVE_EXTENSION TARGET_FILE LAST_ONLY)
        message (STATUS "Generating ${TARGET_FILE}...")

        set (CONTENT "")
        include (${GENERATOR})
        write_if_not_equal ("${TARGET_FILE}" "${CONTENT}")
    endforeach ()
endfunction ()

# Recursively finds all targets that are linked to given target.
# Must be called after all linked targets are guarantied to be declared.
function (find_linked_targets_recursively TARGET OUTPUT)
    set (ALL_LINKED_TARGETS)
    set (SCAN_QUEUE)
    list (APPEND SCAN_QUEUE ${TARGET})
    list (LENGTH SCAN_QUEUE SCAN_QUEUE_LENGTH)

    while (SCAN_QUEUE_LENGTH GREATER 0)
        list (POP_BACK SCAN_QUEUE ITEM)
        if (TARGET ${ITEM})
            get_target_property (LINKED_TARGETS ${ITEM} INTERFACE_LINK_LIBRARIES)
            foreach (LINKED_TARGET ${LINKED_TARGETS})
                # Cleanup link-only generator specifications.
                if (LINKED_TARGET MATCHES "^\\$<LINK_ONLY:")
                    string (LENGTH "${LINKED_TARGET}" LINKED_TARGET_LENGTH)
                    math (EXPR NEW_LENGTH "${LINKED_TARGET_LENGTH} - 13")
                    string (SUBSTRING "${LINKED_TARGET}" 12 ${NEW_LENGTH} LINKED_TARGET)
                endif ()

                if (TARGET ${LINKED_TARGET})
                    list (FIND ALL_LINKED_TARGETS "${LINKED_TARGET}" LINKED_TARGET_INDEX)
                    if (LINKED_TARGET_INDEX EQUAL -1)
                        list (APPEND ALL_LINKED_TARGETS ${LINKED_TARGET})
                        list (APPEND SCAN_QUEUE ${LINKED_TARGET})
                    endif ()
                endif ()
            endforeach ()
        endif ()

        list (LENGTH SCAN_QUEUE SCAN_QUEUE_LENGTH)
    endwhile ()

    set ("${OUTPUT}" "${ALL_LINKED_TARGETS}" PARENT_SCOPE)
endfunction ()
