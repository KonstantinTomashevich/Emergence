register_concrete (Handling)
concrete_include (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")
concrete_require (SCOPE PRIVATE ABSTRACT Assert)
