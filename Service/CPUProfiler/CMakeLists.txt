sober_service_begin (CPUProfiler)
    sober_naming_variant_target (APICommon Original API_COMMON_TARGET)
    sober_service_add_api_dependency (${API_COMMON_TARGET})

    sober_service_include_directory ("${CMAKE_CURRENT_SOURCE_DIR}/API")
    sober_service_add_implementation ("${CMAKE_CURRENT_SOURCE_DIR}/Implementation/None")
    sober_service_add_implementation ("${CMAKE_CURRENT_SOURCE_DIR}/Implementation/Tracy")
    sober_service_set_default_implementation (None)
sober_service_end ()
