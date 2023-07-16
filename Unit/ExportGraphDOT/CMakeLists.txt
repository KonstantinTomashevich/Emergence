register_concrete (ExportGraphDOT)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")
concrete_require (SCOPE PRIVATE ABSTRACT ExportGraph Log)
