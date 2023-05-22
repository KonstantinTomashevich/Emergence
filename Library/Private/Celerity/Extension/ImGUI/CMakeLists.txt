sober_library_begin (CelerityImGUI STATIC)
    file (GLOB_RECURSE HEADERS "Public/*.hpp")
    file (GLOB_RECURSE SOURCES "Public/*.cpp")
    file (GLOB_RECURSE SHADERS "Resources/Shaders/*.sc")

    list (APPEND SOURCES ${HEADERS})
    list (APPEND SOURCES ${SHADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Public")

    sober_library_use_service (EXPECTED PRIVATE JobDispatcher)
    sober_library_use_service (EXPECTED PRIVATE Log)

    sober_naming_service_target (CelerityUI CELERITY_UI_API_TARGET)
    sober_naming_variant_target (Celerity Original CELERITY_LIBRARY_TARGET)
    sober_naming_variant_target (CelerityAsset Original CELERITY_ASSET_LIBRARY_TARGET)
    sober_naming_variant_target (CelerityLocale Original CELERITY_LOCALE_LIBRARY_TARGET)
    sober_naming_variant_target (CelerityRenderFoundation Original CELERITY_RENDER_FOUNDATION_LIBRARY_TARGET)

    sober_library_link_library (PRIVATE "${CELERITY_ASSET_LIBRARY_TARGET}")
    sober_library_link_library (PRIVATE "${CELERITY_LOCALE_LIBRARY_TARGET}")
    sober_library_link_library (PRIVATE "${CELERITY_RENDER_FOUNDATION_LIBRARY_TARGET}")
    sober_library_link_library (PRIVATE "${CELERITY_UI_API_TARGET}")
    sober_library_link_library (PRIVATE ImGUI)
    sober_library_link_library (PUBLIC "${CELERITY_LIBRARY_TARGET}")

    sober_variant_begin (Original)
    sober_variant_end ()
sober_library_end ()

set (RESOURCES_COMPILED_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Resources")
file (MAKE_DIRECTORY "${RESOURCES_COMPILED_DIRECTORY}")
file (CREATE_LINK "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Content" "${RESOURCES_COMPILED_DIRECTORY}/Content" SYMBOLIC)
register_bgfx_shaders ("${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shaders" "${RESOURCES_COMPILED_DIRECTORY}/Shaders")

sober_naming_library_base_target (CelerityImGUI CELERITY_IMGUI_LIBRARY)
register_resource_usage ("${CELERITY_IMGUI_LIBRARY}" "${RESOURCES_COMPILED_DIRECTORY}" "ImGUIResources")
