register_concrete (SDLContextHolder)
concrete_include (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")
concrete_require (SCOPE PRIVATE ABSTRACT RenderBackend INTERFACE Testing THIRD_PARTY SDL3::SDL3)
