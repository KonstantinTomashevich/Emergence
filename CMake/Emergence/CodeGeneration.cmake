# Executes CMake-driven code generation by including all "*.generated.*.cmake" files.
function (call_code_generation ROOT_DIRECTORY)
    file (GLOB_RECURSE GENERATORS "${ROOT_DIRECTORY}/*.generated.*.cmake")
    foreach (GENERATOR IN LISTS GENERATORS)
        set (TARGET_FILE "${GENERATOR}")
        cmake_path (REMOVE_EXTENSION TARGET_FILE LAST_ONLY)
        message (STATUS "Generating ${TARGET_FILE}...")

        set (CONTENT "")
        include (${GENERATOR})
        file_write_if_not_equal ("${TARGET_FILE}" "${CONTENT}")
    endforeach ()
endfunction ()
