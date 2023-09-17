register_abstract (Memory)
abstract_include ("${CMAKE_CURRENT_SOURCE_DIR}")
abstract_require (ABSTRACT MemoryProfiler INTERFACE APICommon)
abstract_register_implementation (NAME Original PARTS MemoryOriginal)
