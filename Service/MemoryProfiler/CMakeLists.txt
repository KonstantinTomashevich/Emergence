sober_service_begin (MemoryProfiler)
    sober_naming_variant_target (APICommon Original API_COMMON_TARGET)
    sober_naming_service_target (Memory MEMORY_API_TARGET)

    sober_service_add_api_dependency (${API_COMMON_TARGET})
    sober_service_add_api_dependency (${MEMORY_API_TARGET})

    sober_service_include_directory ("${CMAKE_CURRENT_SOURCE_DIR}/API")
    sober_service_add_implementation ("${CMAKE_CURRENT_SOURCE_DIR}/Implementation/None")
    sober_service_add_implementation ("${CMAKE_CURRENT_SOURCE_DIR}/Implementation/Original")
    sober_service_set_default_implementation (Original)
sober_service_end ()
