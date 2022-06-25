add_subdirectory (Constants)

sober_library_begin (Celerity STATIC)
    file (GLOB_RECURSE HEADERS "Public/*.hpp")
    file (GLOB_RECURSE SOURCES "Public/*.cpp")
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Public")

    sober_naming_variant_target (Flow Original FLOW_LIBRARY_TARGET)
    sober_naming_variant_target (Handling Original HANDLING_LIBRARY_TARGET)
    sober_naming_variant_target (Math Original MATH_LIBRARY_TARGET)
    sober_naming_variant_target (SyntaxSugar Original SYNTAX_SUGAR_LIBRARY_TARGET)

    sober_library_link_library (PUBLIC ${FLOW_LIBRARY_TARGET})
    sober_library_link_library (PUBLIC ${HANDLING_LIBRARY_TARGET})
    sober_library_link_library (PUBLIC ${MATH_LIBRARY_TARGET})
    sober_library_link_library (PUBLIC ${SYNTAX_SUGAR_LIBRARY_TARGET})

    sober_library_use_service (EXPECTED PRIVATE Log)
    sober_library_use_service (EXPECTED PRIVATE Memory)
    sober_library_use_service (EXPECTED PRIVATE MemoryProfiler)
    sober_library_use_service (EXPECTED PUBLIC TaskExecutor)
    sober_library_use_service (EXPECTED PUBLIC Warehouse)
    sober_library_use_service (PER_VARIANT PUBLIC CelerityConstants)

    sober_variant_begin (Original)
    sober_variant_end ()
sober_library_end ()
