# Executes CMake-driven code generation by including all "*.generated.*.cmake" files.
function (call_code_generation ROOT_DIRECTORY)
    file (GLOB_RECURSE GENERATORS "${ROOT_DIRECTORY}/*.generated.*.cmake")
    foreach (GENERATOR IN LISTS GENERATORS)
        set (TARGET_FILE "${GENERATOR}")
        cmake_path (REMOVE_EXTENSION TARGET_FILE LAST_ONLY)
        message (STATUS "Generating ${TARGET_FILE}...")

        set (CONTENT "")
        include (${GENERATOR})
        set (CURRENT_CONTENT)

        if (EXISTS "${TARGET_FILE}")
            file (READ "${TARGET_FILE}" CURRENT_CONTENT)
        endif ()

        if (NOT CONTENT STREQUAL CURRENT_CONTENT)
            file (WRITE "${TARGET_FILE}" "${CONTENT}")
        endif ()
    endforeach ()
endfunction ()
