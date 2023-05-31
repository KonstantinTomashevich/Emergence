sober_service_begin (ResourceProvider)
    sober_naming_variant_target (APICommon Original API_COMMON_TARGET)
    sober_naming_variant_target (Container Original CONTAINER_TARGET)

    sober_service_add_api_dependency (${API_COMMON_TARGET})
    sober_service_add_api_dependency (${CONTAINER_TARGET})

    sober_service_include_directory ("${CMAKE_CURRENT_SOURCE_DIR}/API")
    sober_service_add_implementation ("${CMAKE_CURRENT_SOURCE_DIR}/Implementation/Original")
    sober_service_set_default_implementation (Original)
sober_service_end ()
