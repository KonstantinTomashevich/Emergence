# Boost: we use Boost Unordered containers instead of STL because they have important features that
# aren't fully supported by STL. For example, transient hashers and equality checkers: they are in
# C++ 20, but aren't supported yet in a lot of places.
add_subdirectory (boost)

# Box2d
set (BOX2D_BUILD_UNIT_TESTS OFF CACHE BOOL
        "We use double-set with cache to override third party project options for good.")
set (BOX2D_BUILD_UNIT_TESTS OFF)

set (BOX2D_BUILD_TESTBED OFF CACHE BOOL
        "We use double-set with cache to override third party project options for good.")
set (BOX2D_BUILD_TESTBED OFF)

set (BOX2D_USER_SETTINGS ON CACHE BOOL "We use double-set with cache to override third party project options for good.")
set (BOX2D_USER_SETTINGS ON)
set (BUILD_SHARED_LIBS ON)

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

unset (BUILD_SHARED_LIBS)

# CGLM
add_subdirectory (cglm)

# DebugBreak
add_library (DebugBreak INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/debugbreak/debugbreak.h")
target_include_directories (DebugBreak INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/debugbreak")

# Doctest
add_subdirectory (doctest)

# ImGUI
# ImGUI is not using CMake, therefore we need to add our own library targets:
# - Shared library without backend implementation.
# - Static with SDL3 backend implementation for tools.
add_library (ImGUI SHARED
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp")

file (MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/ImGUI/Generated")
generate_api_header (
        API_MACRO ImGUIApi
        EXPORT_MACRO ImGUIImplementation
        OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/ImGUI/Generated/ImGUIApi.h")

target_compile_definitions (ImGUI PRIVATE "ImGUIImplementation")
target_compile_definitions (ImGUI PUBLIC "IMGUI_USER_CONFIG=<EmergenceImGUIConfig.hpp>")

target_include_directories (ImGUI  PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui"
        "${CMAKE_CURRENT_SOURCE_DIR}/ImGUISettingsInjection"
        "${CMAKE_CURRENT_BINARY_DIR}/ImGUI/Generated")

add_library (SDL3ImGUI STATIC
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_sdl3.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_sdlrenderer3.cpp")

target_include_directories (SDL3ImGUI PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends")
target_link_libraries (SDL3ImGUI PUBLIC SDL3::SDL3)

# SDL3
# TODO: Takes ~75% configuration time on CI. Can it be optimized somehow?
set (SDL_SHARED ON CACHE BOOL "We use double-set with cache to override third party project options for good.")
set (SDL_SHARED ON)
add_subdirectory (SDL)

# SPDLog
# We don't need exceptions neither from spdlog nor fmt.
add_compile_definitions (FMT_EXCEPTIONS=0)
set (SPDLOG_NO_EXCEPTIONS ON)
add_subdirectory (spdlog)

# Tracy profiler
set (TRACY_ON_DEMAND ON)
add_subdirectory (tracy)

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

# BGFX
# We add it last due to link libraries addition below.

if (NOT MSVC)
    # On GCC we need to link STL and math everywhere.
    # It seems easier to do it globally here instead of adding this check to every target.
    # Normally, we wouldn't need this for third party targets, but some of them need this
    # dependencies while not linking them (like bgfx sprirv compiler).
    link_libraries (m stdc++)
endif ()

add_subdirectory (bgfx.cmake)
