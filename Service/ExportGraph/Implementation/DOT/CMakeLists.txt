sober_implementation_begin (DOT)
    file (GLOB_RECURSE SOURCES *.cpp)
    file (GLOB_RECURSE HEADERS *.hpp)
    list (APPEND SOURCES ${HEADERS})

    sober_implementation_setup_target (OBJECT "${SOURCES}")
    sober_implementation_include_directory ("${CMAKE_CURRENT_SOURCE_DIR}")

    sober_naming_variant_target (GraphDOTExporter Original GRAPH_DOT_EXPORTER_TARGET)
    sober_implementation_link_library (${GRAPH_DOT_EXPORTER_TARGET})
sober_implementation_end ()
