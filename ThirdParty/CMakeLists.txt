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
if (UNIX)
    # Right now SDL2 from GitHub does not have correct SDL_config.h for Linux for some reason.
    # Perhaps, it is not added to GitHub version as there is lots of distros and it is impossible
    # to create configs for all of them. Therefore on Linux we have to resort to using installed
    # package, that is guaranteed to have correct config.
    find_package (SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2 SDL2main)
else ()
    # TODO: Takes ~75% configuration time on CI. Can it be optimized somehow?
    set (SDL_SHARED ON CACHE BOOL "We use double-set with cache to override third party project options for good.")
    set (SDL_SHARED ON)
    add_subdirectory (SDL)
endif ()

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
