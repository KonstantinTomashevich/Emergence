sober_library_begin ("${GAME_NAME}Extension" STATIC)
    file (GLOB_RECURSE HEADERS *.hpp)
    file (GLOB_RECURSE SOURCES *.cpp)
    list (APPEND SOURCES ${HEADERS})
    
    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

    sober_naming_variant_target (Flow Original FLOW_LIBRARY_TARGET)
    sober_library_link_library (PUBLIC ${FLOW_LIBRARY_TARGET})

    sober_naming_variant_target (SyntaxSugar Original SYNTAX_SUGAR_LIBRARY_TARGET)
    sober_library_link_library (PUBLIC ${SYNTAX_SUGAR_LIBRARY_TARGET})

    sober_library_use_service (PER_VARIANT PUBLIC TaskExecutor)
    sober_library_use_service (PER_VARIANT PUBLIC Warehouse)

    sober_variant_begin (Original)
    sober_variant_end ()
sober_library_end ()
