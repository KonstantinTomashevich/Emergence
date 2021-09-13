sober_library_begin (RecordCollectionVisualization STATIC)
    file (GLOB_RECURSE HEADERS *.hpp)
    file (GLOB_RECURSE SOURCES *.cpp)
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

    # We link only API headers instead of using whole service to make
    # sure that visualization will always use provided implementation.
    sober_naming_service_target (RecordCollection RECORD_COLLECTION_TARGET)
    sober_library_link_library (PUBLIC ${RECORD_COLLECTION_TARGET})

    sober_naming_variant_target (VisualGraph Original VISUAL_GRAPH_LIBRARY_TARGET)
    sober_library_link_library (PUBLIC ${VISUAL_GRAPH_LIBRARY_TARGET})

    sober_variant_begin (Original)
    sober_variant_end ()
sober_library_end ()
