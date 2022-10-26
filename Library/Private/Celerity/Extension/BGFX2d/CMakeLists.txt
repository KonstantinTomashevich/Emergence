sober_library_begin (CelerityBGFX2d STATIC)
    file (GLOB_RECURSE HEADERS "Public/*.hpp" )
    file (GLOB_RECURSE SOURCES "Public/*.cpp")
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

    sober_library_use_service (EXPECTED PRIVATE Log)

    sober_naming_service_target (CelerityRender2d CELERITY_RENDER_2D_API_TARGET)
    sober_naming_variant_target (Celerity Original CELERITY_LIBRARY_TARGET)
    sober_naming_variant_target (CelerityTransform Original CELERITY_TRANSFORM_LIBRARY_TARGET)

    sober_library_link_library (PRIVATE "${CELERITY_RENDER_2D_API_TARGET}")
    sober_library_link_library (PRIVATE "${CELERITY_TRANSFORM_LIBRARY_TARGET}")
    sober_library_link_library (PRIVATE "bgfx")
    sober_library_link_library (PRIVATE "bimg")
    sober_library_link_library (PRIVATE "bx")
    sober_library_link_library (PUBLIC "${CELERITY_LIBRARY_TARGET}")

    sober_variant_begin (Original)
    sober_variant_end ()
sober_library_end ()

sober_naming_library_base_target (CelerityBGFX2d CELERITY_BGFX_2D_LIBRARY)
register_asset_usage ("${CELERITY_BGFX_2D_LIBRARY}" "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Compiled" "BGFX2dAssets")
