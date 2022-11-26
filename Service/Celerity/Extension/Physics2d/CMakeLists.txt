sober_service_begin (CelerityPhysics2d)
    sober_naming_variant_target (Celerity Original CELERITY_TARGET)

    sober_service_add_api_dependency (${CELERITY_TARGET})

    sober_service_include_directory ("${CMAKE_CURRENT_SOURCE_DIR}/API")
    sober_service_add_implementation ("${CMAKE_CURRENT_SOURCE_DIR}/Implementation/Box2d")
    sober_service_set_default_implementation (Box2d)
sober_service_end ()
