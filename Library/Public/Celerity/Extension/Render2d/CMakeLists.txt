sober_library_begin (CelerityRender2d STATIC)
    file (GLOB_RECURSE HEADERS "Public/*.hpp" "Private/*.hpp")
    file (GLOB_RECURSE SOURCES "Public/*.cpp" "Private/*.cpp")
    file (GLOB_RECURSE SHADERS "Resources/Shaders/*.sc")

    list (APPEND SOURCES ${HEADERS})
    list (APPEND SOURCES ${SHADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Private")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Public")

    sober_library_use_service (EXPECTED PRIVATE Log)
    sober_library_use_service (EXPECTED PUBLIC RenderBackend)

    sober_naming_variant_target (Celerity Original CELERITY_LIBRARY_TARGET)
    sober_naming_variant_target (CelerityAsset Original CELERITY_ASSET_LIBRARY_TARGET)
    sober_naming_variant_target (CelerityTransform Original CELERITY_TRANSFORM_LIBRARY_TARGET)
    sober_naming_variant_target (Serialization Original SERIALIZATION_LIBRARY_TARGET)

    sober_library_link_library (PRIVATE "${SERIALIZATION_LIBRARY_TARGET}")
    sober_library_link_library (PUBLIC "${CELERITY_ASSET_LIBRARY_TARGET}")
    sober_library_link_library (PUBLIC "${CELERITY_LIBRARY_TARGET}")
    sober_library_link_library (PUBLIC "${CELERITY_TRANSFORM_LIBRARY_TARGET}")

    sober_variant_begin (Original)
    sober_variant_end ()
sober_library_end ()

set (RESOURCES_COMPILED_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Resources")
file (MAKE_DIRECTORY "${RESOURCES_COMPILED_DIRECTORY}")
file (CREATE_LINK "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Materials"
        "${RESOURCES_COMPILED_DIRECTORY}/Materials" SYMBOLIC)
register_bgfx_shaders ("${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shaders" "${RESOURCES_COMPILED_DIRECTORY}/Shaders")

sober_naming_library_base_target (CelerityRender2d CELERITY_RENDER_2D_LIBRARY)
register_resource_usage ("${CELERITY_RENDER_2D_LIBRARY}" "${RESOURCES_COMPILED_DIRECTORY}" "Render2dResources")
