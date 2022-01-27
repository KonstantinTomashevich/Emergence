sober_service_begin (Warehouse)
    sober_naming_variant_target (APICommon Original API_COMMON_TARGET)
    sober_naming_variant_target (Container Original CONTAINER_TARGET)
    sober_naming_service_target (Memory MEMORY_TARGET)
    sober_naming_service_target (StandardLayoutMapping STANDARD_LAYOUT_MAPPING_TARGET)
    sober_naming_variant_target (VisualGraph Original VISUAL_GRAPH_TARGET)

    sober_service_add_api_dependency (${API_COMMON_TARGET})
    sober_service_add_api_dependency (${CONTAINER_TARGET})
    sober_service_add_api_dependency (${MEMORY_TARGET})
    sober_service_add_api_dependency (${STANDARD_LAYOUT_MAPPING_TARGET})
    sober_service_add_api_dependency (${VISUAL_GRAPH_TARGET})

    sober_service_include_directory ("${CMAKE_CURRENT_SOURCE_DIR}/API")
    sober_service_add_implementation ("${CMAKE_CURRENT_SOURCE_DIR}/Implementation/GalleonOriginal")
    sober_service_set_default_implementation (GalleonOriginal)
sober_service_end ()

add_subdirectory (Visualization)
