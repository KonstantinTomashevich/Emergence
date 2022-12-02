# We cannot build this library unless PhysX is found.
if (NOT TARGET PhysX)
    return ()
endif ()

sober_library_begin (CelerityPhysX STATIC)
    file (GLOB_RECURSE HEADERS "*.hpp" )
    file (GLOB_RECURSE SOURCES "*.cpp")
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

    sober_library_use_service (EXPECTED PRIVATE JobDispatcher)
    sober_library_use_service (EXPECTED PRIVATE Log)

    sober_naming_service_target (CelerityPhysics3d CELERITY_PHYSICS_3D_API_TARGET)
    sober_naming_variant_target (Celerity Original CELERITY_LIBRARY_TARGET)
    sober_naming_variant_target (CelerityTransform Original CELERITY_TRANSFORM_LIBRARY_TARGET)

    sober_library_link_library (PRIVATE "${CELERITY_PHYSICS_3D_API_TARGET}")
    sober_library_link_library (PRIVATE "${CELERITY_TRANSFORM_LIBRARY_TARGET}")
    sober_library_link_library (PRIVATE PhysX)
    sober_library_link_library (PUBLIC "${CELERITY_LIBRARY_TARGET}")

    sober_variant_begin (Original)
    sober_variant_end ()
sober_library_end ()
