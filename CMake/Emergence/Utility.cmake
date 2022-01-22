# Adds custom command that copies shared libraries, explicitly requested by given target.
function (copy_required_shared_libraries TARGET)
    add_custom_command (
            TARGET "${TARGET}" POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:${TARGET}> $<TARGET_FILE_DIR:${TARGET}>
            COMMAND_EXPAND_LISTS)
endfunction ()