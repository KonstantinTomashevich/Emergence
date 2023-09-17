register_abstract (ExportGraph)
abstract_include ("${CMAKE_CURRENT_SOURCE_DIR}")
abstract_require (CONCRETE_INTERFACE VisualGraph)
abstract_register_implementation (NAME DOT PARTS ExportGraphDOT)
