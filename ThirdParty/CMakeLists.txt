add_subdirectory (cglm)
add_subdirectory (doctest)

# ImGUI is not using CMake, therefore we need to add our own library target.
add_library (SDL2ImGUI STATIC
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_sdl.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_sdlrenderer.cpp")

target_include_directories (SDL2ImGUI PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends")
target_link_libraries (SDL2ImGUI PUBLIC SDL2)

# TODO: Takes ~75% configuration time on CI. Can it be optimized somehow?
add_subdirectory (SDL)

# We don't need exceptions neither from spdlog nor fmt.
add_compile_definitions (FMT_EXCEPTIONS=0)
set (SPDLOG_NO_EXCEPTIONS ON)
add_subdirectory (spdlog)

add_subdirectory (xxhash_cpp)
