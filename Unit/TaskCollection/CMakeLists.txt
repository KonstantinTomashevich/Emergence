register_concrete (TaskCollection)
concrete_include (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")
concrete_require (SCOPE PUBLIC ABSTRACT Memory CONCRETE_INTERFACE Container)
