sober_library_begin (CelerityBGFX2d STATIC)
    file (GLOB_RECURSE HEADERS "Public/*.hpp" )
    file (GLOB_RECURSE SOURCES "Public/*.cpp")
    file (GLOB_RECURSE SHADERS "Assets/Shaders/*.sc")

    list (APPEND SOURCES ${HEADERS})
    list (APPEND SOURCES ${SHADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Public")

    sober_library_use_service (EXPECTED PRIVATE Log)

    sober_naming_service_target (CelerityRender2d CELERITY_RENDER_2D_API_TARGET)
    sober_naming_variant_target (Celerity Original CELERITY_LIBRARY_TARGET)
    sober_naming_variant_target (CelerityTransform Original CELERITY_TRANSFORM_LIBRARY_TARGET)
    sober_naming_variant_target (Serialization Original SERIALIZATION_LIBRARY_TARGET)

    sober_library_link_library (PRIVATE "${CELERITY_LIBRARY_TARGET}")
    sober_library_link_library (PRIVATE "${CELERITY_RENDER_2D_API_TARGET}")
    sober_library_link_library (PRIVATE "${CELERITY_TRANSFORM_LIBRARY_TARGET}")
    sober_library_link_library (PRIVATE "${SERIALIZATION_LIBRARY_TARGET}")
    sober_library_link_library (PRIVATE "bgfx")
    sober_library_link_library (PRIVATE "bimg")
    sober_library_link_library (PRIVATE "bx")

    sober_variant_begin (Original)
    sober_variant_end ()
sober_library_end ()

set (ASSETS_COMPILED_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Assets")
file (CREATE_LINK "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Materials" "${ASSETS_COMPILED_DIRECTORY}/Materials" SYMBOLIC)
register_bgfx_shaders ("${CMAKE_CURRENT_SOURCE_DIR}/Assets/Shaders" "${ASSETS_COMPILED_DIRECTORY}/Shaders")
sober_naming_library_base_target (CelerityBGFX2d CELERITY_BGFX_2D_LIBRARY)
register_asset_usage ("${CELERITY_BGFX_2D_LIBRARY}" "${ASSETS_COMPILED_DIRECTORY}" "BGFX2dAssets")
