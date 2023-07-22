register_concrete (${GAME_NAME}GameApplication)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")

concrete_require (SCOPE PRIVATE
        ABSTRACT CelerityPhysics2dLogic CelerityUILogic Log RenderBackend

        CONCRETE_INTERFACE
        ${GAME_NAME}Logic ${GAME_NAME}Model ${GAME_NAME}Resources CelerityLocaleLogic CelerityPhysics2dDebugDrawLogic
        CelerityRender2dLogic CelerityRenderFoundationLogic CelerityResourceObjectLogic CelerityTransformLogic
        MemoryRecording Serialization Time

        INTERFACE ResourceProviderHelpers VirtualFileSystemHelpers
        THIRD_PARTY SDL3::SDL3)
