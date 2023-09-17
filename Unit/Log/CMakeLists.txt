register_abstract (Log)
abstract_include ("${CMAKE_CURRENT_SOURCE_DIR}")
abstract_require (ABSTRACT Memory CONCRETE_INTERFACE Container INTERFACE APICommon)
abstract_register_implementation (NAME SPDLog PARTS LogSPDLog)
