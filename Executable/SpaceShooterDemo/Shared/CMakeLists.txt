sober_library_begin ("${GAME_NAME}Shared" STATIC)
    file (GLOB_RECURSE HEADERS *.hpp)
    file (GLOB_RECURSE SOURCES *.cpp)
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

    sober_naming_variant_target ("${GAME_NAME}Model" Original MODEL_TARGET)
    sober_naming_variant_target (CelerityAssembly Original CELERITY_ASSEMBLY_TARGET)
    sober_naming_variant_target (CelerityInput Original CELERITY_INPUT_TARGET)
    sober_naming_variant_target (CelerityTransform Original CELERITY_TRANSFORM_TARGET)

    sober_library_link_library (PUBLIC ${CELERITY_ASSEMBLY_TARGET})
    sober_library_link_library (PUBLIC ${CELERITY_INPUT_TARGET})
    sober_library_link_library (PUBLIC ${CELERITY_TRANSFORM_TARGET})
    sober_library_link_library (PUBLIC ${MODEL_TARGET})
    sober_library_link_library (PUBLIC Urho3D)

    sober_library_use_service (EXPECTED PRIVATE Log)
    sober_library_use_service (EXPECTED PRIVATE Memory)
    sober_library_use_service (EXPECTED PRIVATE MemoryProfiler)
    sober_library_use_service (EXPECTED PUBLIC CelerityPhysics3d)

    sober_variant_begin (Original)
    sober_variant_end ()
sober_library_end ()
