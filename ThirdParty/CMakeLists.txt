# BGFX
add_subdirectory (bgfx.cmake)

# Box2d
set (BOX2D_BUILD_UNIT_TESTS OFF CACHE BOOL
        "We use double-set with cache to override third party project options for good.")
set (BOX2D_BUILD_UNIT_TESTS OFF)

set (BOX2D_BUILD_TESTBED OFF CACHE BOOL
        "We use double-set with cache to override third party project options for good.")
set (BOX2D_BUILD_TESTBED OFF)

set (BOX2D_USER_SETTINGS ON CACHE BOOL "We use double-set with cache to override third party project options for good.")
set (BOX2D_USER_SETTINGS ON)

add_subdirectory (box2d)
target_sources (box2d PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Box2dSettingsInjection/b2_user_settings.cpp")

# Box2d installation configuration makes it impossible to include Box2dSettingsInjection,
# therefore we need to do "a sneaky copy" in order to pass our settings.
add_custom_target (InjectBox2dUserSettingsHeader
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/Box2dSettingsInjection/b2_user_settings.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/box2d/include/box2d"
        COMMENT "Copying user settings to box2d include path...")
add_dependencies (box2d InjectBox2dUserSettingsHeader)
add_dependencies (BuildTimeCodeGeneration InjectBox2dUserSettingsHeader)

# CGLM
add_subdirectory (cglm)

# DebugBreak
add_library (DebugBreak INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/debugbreak/debugbreak.h")
target_include_directories (DebugBreak INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/debugbreak")

# Doctest
add_subdirectory (doctest)

# ImGUI
# ImGUI is not using CMake, therefore we need to add our own library target.
add_library (ImGUI STATIC
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp")

target_include_directories (ImGUI PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/imgui")

add_library (SDL2ImGUI STATIC
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_sdl.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_sdlrenderer.cpp")

target_include_directories (SDL2ImGUI PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends")
target_link_libraries (SDL2ImGUI PUBLIC ImGUI SDL2)

# SDL2
# TODO: Takes ~75% configuration time on CI. Can it be optimized somehow?
set (SDL_SHARED ON CACHE BOOL "We use double-set with cache to override third party project options for good.")
set (SDL_SHARED ON)
add_subdirectory (SDL)

# SPDLog
# We don't need exceptions neither from spdlog nor fmt.
add_compile_definitions (FMT_EXCEPTIONS=0)
set (SPDLOG_NO_EXCEPTIONS ON)
add_subdirectory (spdlog)

# XXHash
add_subdirectory (xxhash_cpp)

# YAML-Cpp
set (YAML_CPP_BUILD_TOOLS OFF CACHE BOOL
        "We use double-set with cache to override third party project options for good.")
set (YAML_CPP_BUILD_TOOLS OFF)
add_subdirectory (yaml-cpp)

# On MSVC CLang exceptions are disabled by default, but yaml-cpp is not compiling without them.
if (MSVC)
    target_compile_options (yaml-cpp PRIVATE /EHa)
endif ()
