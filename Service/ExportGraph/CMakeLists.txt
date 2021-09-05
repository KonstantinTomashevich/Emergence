sober_service_begin (ExportGraph)
    sober_naming_variant_target (VisualGraph Original VISUAL_GRAPH_TARGET)
    sober_service_add_api_dependency (${VISUAL_GRAPH_TARGET})

    sober_service_include_directory ("${CMAKE_CURRENT_SOURCE_DIR}/API")
    sober_service_add_implementation ("${CMAKE_CURRENT_SOURCE_DIR}/Implementation/DOT")
    sober_service_set_default_implementation (DOT)
sober_service_end ()
