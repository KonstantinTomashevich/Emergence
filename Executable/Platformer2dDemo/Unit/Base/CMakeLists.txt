register_concrete (${GAME_NAME}Base)
concrete_include (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")
concrete_require (SCOPE PRIVATE THIRD_PARTY SDL3::SDL3)

concrete_require (
        SCOPE PUBLIC
        ABSTRACT ResourceProvider VirtualFileSystem
        CONCRETE_INTERFACE Celerity CelerityNexus InputStorage)
