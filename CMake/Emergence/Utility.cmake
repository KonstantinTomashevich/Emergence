# Traverses linked libraries tree of given target and outputs all shared libraries to output list with given name.
function (find_required_shared_libraries TARGET OUTPUT_VARIABLE)
    set (LIBRARIES)
    sober_find_linked_targets_recursively ("${TARGET}" ALL_LINKED_TARGETS)

    foreach (LINKED_TARGET ${ALL_LINKED_TARGETS})
        get_target_property (TARGET_TYPE "${LINKED_TARGET}" TYPE)
        if (TARGET_TYPE STREQUAL "SHARED_LIBRARY")
            list (APPEND LIBRARIES "${LINKED_TARGET}")
        endif ()
    endforeach ()

    set ("${OUTPUT_VARIABLE}" "${LIBRARIES}" PARENT_SCOPE)
endfunction ()

# Adds custom command that copies shared libraries, requested by given target.
# On Unix platforms also forces Windows-like SO search behaviour by settings build rpath to $ORIGIN.
function (copy_required_shared_libraries TARGET)
    find_required_shared_libraries ("${TARGET}" REQUIRED_LIBRARIES)
    foreach (LIBRARY_TARGET ${REQUIRED_LIBRARIES})
        if (UNIX)
            add_custom_command (
                    TARGET "${TARGET}" POST_BUILD
                    COMMAND
                    ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_SONAME_FILE:${LIBRARY_TARGET}> $<TARGET_FILE_DIR:${TARGET}>
                    COMMAND_EXPAND_LISTS)
        else ()
            add_custom_command (
                    TARGET "${TARGET}" POST_BUILD
                    COMMAND
                    ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE:${LIBRARY_TARGET}> $<TARGET_FILE_DIR:${TARGET}>
                    COMMAND_EXPAND_LISTS)
        endif ()
    endforeach ()

    # Force Windows-like behaviour on rpath-driven unix builds.
    if (UNIX)
        set_target_properties ("${TARGET}" PROPERTIES BUILD_RPATH "\$ORIGIN")
    endif ()
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
