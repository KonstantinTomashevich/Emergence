if (NOT EMERGENCE_INCLUDE_GPU_DEPENDANT_TESTS)
    return ()
endif ()

file (GLOB_RECURSE SOURCES *.cpp)
file (GLOB_RECURSE HEADERS *.hpp)
file (GLOB_RECURSE SHADERS *.sc)

add_executable (TestCelerityRender2dOriginal ${SOURCES} ${HEADERS} ${SHADERS})
add_test (NAME "TestCelerityRender2dOriginal" COMMAND TestCelerityRender2dOriginal)
add_dependencies (LibraryTests TestCelerityRender2dOriginal)
target_include_directories (TestCelerityRender2dOriginal PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

sober_naming_implementation_target (RenderBackend BGFX RENDER_BACKEND_BGFX_TARGET)
sober_naming_implementation_target (ResourceProvider Original RESOURCE_PROVIDER_ORIGINAL_TARGET)
sober_naming_implementation_target (VirtualFileSystem Original VIRTUAL_FILE_SYSTEM_IMPLEMENTATION_TARGET)
sober_naming_variant_target (CelerityRender2d Original CELERITY_RENDER_2D_LIBRARY_TARGET)
sober_naming_variant_target (FileSystemTestUtility Doctest FILE_SYSTEM_TEST_UTILITY_LIBRARY_TARGET)
sober_naming_variant_target (ResourceContextHolder Doctest RESOURCE_CONTEXT_HOLDER_LIBRARY_TARGET)
sober_naming_variant_target (SDLContextHolder Original SDL_CONTEXT_HOLDER_LIBRARY_TARGET)
sober_naming_variant_target (Serialization Original SERIALIZATION_LIBRARY_TARGET)

sober_target_link_libraries (TestCelerityRender2dOriginal PUBLIC
        ${RENDER_BACKEND_BGFX_TARGET}
        ${RESOURCE_PROVIDER_ORIGINAL_TARGET}
        ${VIRTUAL_FILE_SYSTEM_IMPLEMENTATION_TARGET}
        ${CELERITY_RENDER_2D_LIBRARY_TARGET}
        ${FILE_SYSTEM_TEST_UTILITY_LIBRARY_TARGET}
        ${RESOURCE_CONTEXT_HOLDER_LIBRARY_TARGET}
        ${SDL_CONTEXT_HOLDER_LIBRARY_TARGET}
        ${SERIALIZATION_LIBRARY_TARGET}
        SDL3::SDL3)
link_celerity_with_dependencies (TestCelerityRender2dOriginal)

# On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
if (MSVC)
    target_compile_options (TestCelerityRender2dOriginal PRIVATE /EHa)
endif ()

set (RESOURCES_COMPILED_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Resources")
file (MAKE_DIRECTORY "${RESOURCES_COMPILED_DIRECTORY}")
register_bgfx_shaders ("${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shaders" "${RESOURCES_COMPILED_DIRECTORY}/Shaders")

register_resource_usage (
        TestCelerityRender2dOriginal "${RESOURCES_COMPILED_DIRECTORY}" "Render2dTest/Compiled" "CoreResources")

register_resource_usage (
        TestCelerityRender2dOriginal "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Content" "Render2dTest/Raw" "CoreResources")

register_resource_usage (TestCelerityRender2dOriginal
        "${CMAKE_CURRENT_SOURCE_DIR}/Expectation" "Render2dTest/Expectation" "TestResources")

copy_required_shared_libraries (TestCelerityRender2dOriginal)
deploy_resource_mount_lists (TestCelerityRender2dOriginal "${CMAKE_CURRENT_BINARY_DIR}")
