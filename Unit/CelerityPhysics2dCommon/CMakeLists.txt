register_concrete (CelerityPhysics2dCommon)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")
concrete_implements_abstract (CelerityPhysics2d)
