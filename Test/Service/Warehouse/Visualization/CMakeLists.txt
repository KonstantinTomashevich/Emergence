sober_service_begin (TestWarehouseVisualizationGraphs)
    sober_naming_variant_target (WarehouseVisualization Original VISUALIZATION_LIBRARY_TARGET)
    sober_service_add_api_dependency (${VISUALIZATION_LIBRARY_TARGET})

    sober_service_include_directory ("${CMAKE_CURRENT_SOURCE_DIR}/API")
    sober_service_add_implementation ("${CMAKE_CURRENT_SOURCE_DIR}/Implementation/Galleon")
    sober_service_set_default_implementation (Galleon)
sober_service_end ()
