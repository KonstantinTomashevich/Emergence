register_concrete (${GAME_NAME}GameApplication)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")

concrete_require (SCOPE PRIVATE
        ABSTRACT Log RenderBackend
        CONCRETE_INTERFACE ${GAME_NAME}Base ${GAME_NAME}Resources MemoryRecording Serialization Time
        INTERFACE VirtualFileSystemHelpers
        THIRD_PARTY SDL3::SDL3)
