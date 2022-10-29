sober_service_begin (CelerityRender2d)
    sober_naming_variant_target (CelerityAsset Original CELERITY_ASSET_TARGET)
    sober_naming_variant_target (Celerity Original CELERITY_TARGET)

    sober_service_add_api_dependency (${CELERITY_ASSET_TARGET})
    sober_service_add_api_dependency (${CELERITY_TARGET})

    sober_service_include_directory ("${CMAKE_CURRENT_SOURCE_DIR}/API")
    sober_service_add_implementation ("${CMAKE_CURRENT_SOURCE_DIR}/Implementation/BGFX")
    sober_service_set_default_implementation (BGFX)
sober_service_end ()
