sober_service_begin (MemoryPool)
    sober_naming_variant_target (APICommon Original API_COMMON_TARGET)
    sober_service_add_api_dependency (${API_COMMON_TARGET})

    sober_service_include_directory ("${CMAKE_CURRENT_SOURCE_DIR}/API")
    sober_service_add_implementation ("${CMAKE_CURRENT_SOURCE_DIR}/Implementation/Unordered")
    sober_service_set_default_implementation (Unordered)
sober_service_end ()
