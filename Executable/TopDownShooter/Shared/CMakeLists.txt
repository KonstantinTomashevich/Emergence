sober_library_begin ("${GAME_NAME}Shared" STATIC)
    file (GLOB_RECURSE HEADERS *.hpp)
    file (GLOB_RECURSE SOURCES *.cpp)
    list (APPEND SOURCES ${HEADERS})
    
    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
    
    sober_naming_variant_target ("${GAME_NAME}Extension" Original EXTENSION_TARGET)
    sober_library_link_library (PUBLIC ${EXTENSION_TARGET})

    sober_naming_variant_target ("${GAME_NAME}Model" Original MODEL_TARGET)
    sober_library_link_library (PUBLIC ${MODEL_TARGET})

    sober_library_link_library (PUBLIC OgreBites)
    sober_library_link_library (PUBLIC OgreRTShaderSystem)

    sober_library_use_service (EXPECTED PRIVATE Log)

    sober_variant_begin (Original)
    sober_variant_end ()
sober_library_end ()
