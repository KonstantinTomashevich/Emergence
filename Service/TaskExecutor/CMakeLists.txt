sober_service_begin (TaskExecutor)
    sober_naming_variant_target (APICommon Original API_COMMON_TARGET)
    sober_service_add_api_dependency (${API_COMMON_TARGET})

    sober_naming_variant_target (TaskCollection Original TASK_COLLECTION_TARGET)
    sober_service_add_api_dependency (${TASK_COLLECTION_TARGET})

    sober_service_include_directory ("${CMAKE_CURRENT_SOURCE_DIR}/API")
    sober_service_add_implementation ("${CMAKE_CURRENT_SOURCE_DIR}/Implementation/Sequential")
    sober_service_add_implementation ("${CMAKE_CURRENT_SOURCE_DIR}/Implementation/Parallel")
    sober_service_set_default_implementation (Parallel)
sober_service_end ()
