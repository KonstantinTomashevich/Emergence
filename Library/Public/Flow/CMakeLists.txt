add_subdirectory (Constants)

sober_library_begin (Flow STATIC)
    file (GLOB_RECURSE HEADERS "Public/*.hpp")
    file (GLOB_RECURSE SOURCES "Public/*.cpp")
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Public")

    sober_library_use_service (EXPECTED PRIVATE Log)
    sober_library_use_service (PER_VARIANT PRIVATE FlowConstants)

    sober_naming_variant_target (APICommon Original API_COMMON_LIBRARY_TARGET)
    sober_library_link_library (PUBLIC ${API_COMMON_LIBRARY_TARGET})

    sober_naming_variant_target (TaskCollection Original TASK_COLLECTION_LIBRARY_TARGET)
    sober_library_link_library (PUBLIC ${TASK_COLLECTION_LIBRARY_TARGET})

    sober_naming_variant_target (VisualGraph Original VISUAL_GRAPH_LIBRARY_TARGET)
    sober_library_link_library (PUBLIC ${VISUAL_GRAPH_LIBRARY_TARGET})

    sober_variant_begin (Original)
    sober_variant_end ()
sober_library_end ()
