sober_library_begin ("${GAME_NAME}Model" STATIC)
    file (GLOB_RECURSE HEADERS *.hpp)
    file (GLOB_RECURSE SOURCES *.cpp)
    list (APPEND SOURCES ${HEADERS})
    
    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

    sober_naming_variant_target ("${GAME_NAME}Extension" Original EXTENSION_TARGET)
    sober_naming_variant_target (Container Original CONTAINER_LIBRARY_TARGET)
    sober_naming_variant_target (SyntaxSugar Original SYNTAX_SUGAR_LIBRARY_TARGET)

    sober_library_link_library (PUBLIC ${CONTAINER_LIBRARY_TARGET})
    sober_library_link_library (PUBLIC ${EXTENSION_TARGET})
    sober_library_link_library (PUBLIC ${SYNTAX_SUGAR_LIBRARY_TARGET})

    sober_library_use_service (EXPECTED PUBLIC Memory)
    sober_library_use_service (EXPECTED PUBLIC StandardLayoutMapping)

    sober_variant_begin (Original)
    sober_variant_end ()
sober_library_end ()
