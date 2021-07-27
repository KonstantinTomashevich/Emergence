sober_service_begin (RecordCollection)
    sober_naming_variant_target (APICommons Original API_COMMONS_TARGET)
    sober_service_add_api_dependency (${API_COMMONS_TARGET})
    sober_service_add_api_dependency (StandardLayoutMapping)

    sober_service_include_directory ("${CMAKE_CURRENT_SOURCE_DIR}/API")
    sober_service_add_implementation ("${CMAKE_CURRENT_SOURCE_DIR}/Implementation/PegasusOriginal")
    sober_service_set_default_implementation (PegasusOriginal)
sober_service_end ()