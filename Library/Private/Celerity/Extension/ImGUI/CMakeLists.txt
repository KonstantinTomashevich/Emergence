sober_library_begin (CelerityImGUI STATIC)
    file (GLOB_RECURSE HEADERS "*.hpp" )
    file (GLOB_RECURSE SOURCES "*.cpp")
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

    sober_library_use_service (EXPECTED PRIVATE Log)

    sober_naming_service_target (CelerityUI CELERITY_UI_API_TARGET)
    sober_naming_variant_target (Celerity Original CELERITY_LIBRARY_TARGET)
    sober_naming_variant_target (CelerityAsset Original CELERITY_ASSET_LIBRARY_TARGET)
    sober_naming_variant_target (CelerityRenderFoundation Original CELERITY_RENDER_FOUNDATION_LIBRARY_TARGET)

    sober_library_link_library (PRIVATE "${CELERITY_ASSET_TARGET}")
    sober_library_link_library (PRIVATE "${CELERITY_RENDER_FOUNDATION_LIBRARY_TARGET}")
    sober_library_link_library (PRIVATE "${CELERITY_UI_API_TARGET}")
    sober_library_link_library (PRIVATE ImGUI)
    sober_library_link_library (PUBLIC "${CELERITY_LIBRARY_TARGET}")

    sober_variant_begin (Original)
    sober_variant_end ()
sober_library_end ()
