register_concrete (RecordCollectionVisualization)
concrete_include (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")
concrete_require (SCOPE PUBLIC ABSTRACT RecordCollection CONCRETE_INTERFACE SyntaxSugar VisualGraph)
