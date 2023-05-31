sober_library_begin (CelerityRenderFoundation OBJECT)
    file (GLOB_RECURSE HEADERS "Public/*.hpp" "Private/*.hpp")
    file (GLOB_RECURSE SOURCES "Public/*.cpp" "Private/*.cpp")
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Private")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Public")

    sober_library_use_service (EXPECTED PRIVATE JobDispatcher)
    sober_library_use_service (EXPECTED PRIVATE Log)
    sober_library_use_service (EXPECTED PUBLIC RenderBackend)
    sober_library_use_service (EXPECTED PUBLIC ResourceProvider)

    sober_naming_variant_target (Celerity Original CELERITY_LIBRARY_TARGET)
    sober_naming_variant_target (CelerityAsset Original CELERITY_ASSET_LIBRARY_TARGET)
    sober_naming_variant_target (Math Original MATH_LIBRARY_TARGET)

    sober_library_link_library (PUBLIC "${CELERITY_ASSET_LIBRARY_TARGET}")
    sober_library_link_library (PUBLIC "${CELERITY_LIBRARY_TARGET}")
    sober_library_link_library (PUBLIC "${MATH_LIBRARY_TARGET}")

    sober_variant_begin (Original)
    sober_variant_end ()
sober_library_end ()
