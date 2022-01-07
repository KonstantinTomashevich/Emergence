sober_service_begin (TestExportGraphExpectation)
    sober_naming_variant_target (Container Original CONTAINER_TARGET)
    sober_service_add_api_dependency (${CONTAINER_TARGET})

    sober_service_include_directory ("${CMAKE_CURRENT_SOURCE_DIR}/API")
    sober_service_add_implementation ("${CMAKE_CURRENT_SOURCE_DIR}/Implementation/DOT")
    sober_service_set_default_implementation (DOT)
sober_service_end ()
