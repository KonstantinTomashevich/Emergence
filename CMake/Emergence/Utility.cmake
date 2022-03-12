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
