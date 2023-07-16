register_concrete (Container)
concrete_include (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")
concrete_require (SCOPE PUBLIC ABSTRACT Assert Memory StandardLayoutMapping THIRD_PARTY Boost::unordered)
