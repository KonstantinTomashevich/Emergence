sober_library_begin (CelerityPhysics2dDebugDraw STATIC)
    file (GLOB_RECURSE HEADERS "*.hpp")
    file (GLOB_RECURSE SOURCES "*.cpp")
    file (GLOB_RECURSE SHADERS "Resources/Shaders/*.sc")

    list (APPEND SOURCES ${HEADERS})
    list (APPEND SOURCES ${SHADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

    sober_library_use_service (EXPECTED PRIVATE Log)
    sober_library_use_service (EXPECTED PRIVATE CelerityPhysics2d)

    sober_naming_variant_target (Celerity Original CELERITY_LIBRARY_TARGET)
    sober_naming_variant_target (CelerityRender2d Original CELERITY_RENDER_2D_LIBRARY_TARGET)

    sober_library_link_library (PUBLIC "${CELERITY_LIBRARY_TARGET}")
    sober_library_link_library (PUBLIC "${CELERITY_RENDER_2D_LIBRARY_TARGET}")

    sober_variant_begin (Original)
    sober_variant_end ()
sober_library_end ()

set (RESOURCES_COMPILED_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Resources")
file (MAKE_DIRECTORY "${RESOURCES_COMPILED_DIRECTORY}")
file (CREATE_LINK "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Materials"
        "${RESOURCES_COMPILED_DIRECTORY}/Materials" SYMBOLIC)
file (CREATE_LINK "${CMAKE_CURRENT_SOURCE_DIR}/Resources/MaterialInstances"
        "${RESOURCES_COMPILED_DIRECTORY}/MaterialInstances" SYMBOLIC)
register_bgfx_shaders ("${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shaders" "${RESOURCES_COMPILED_DIRECTORY}/Shaders")

sober_naming_library_base_target (CelerityPhysics2dDebugDraw CELERITY_PHYSICS_2D_DEBUG_DRAW_LIBRARY)
register_resource_usage ("${CELERITY_PHYSICS_2D_DEBUG_DRAW_LIBRARY}"
        "${RESOURCES_COMPILED_DIRECTORY}" "Physics2dDebugDrawResources")
