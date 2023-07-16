register_concrete (${GAME_NAME}GameApplication)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")

concrete_require (SCOPE PRIVATE
        ABSTRACT CelerityPhysics2d CelerityUI Log RenderBackend
        CONCRETE_INTERFACE
        ${GAME_NAME}Logic ${GAME_NAME}Model ${GAME_NAME}Resources CelerityLocale CelerityPhysics2dDebugDraw
        CelerityRender2d CelerityRenderFoundation CelerityResourceObject CelerityTransform MemoryRecording
        Serialization
        THIRD_PARTY SDL3::SDL3)
