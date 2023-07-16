register_concrete (StandardLayoutMappingVisualization)
concrete_include (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")
concrete_require (SCOPE PUBLIC ABSTRACT StandardLayoutMapping CONCRETE_INTERFACE VisualGraph)
