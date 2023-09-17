register_abstract (ResourceProvider)
abstract_include ("${CMAKE_CURRENT_SOURCE_DIR}")
abstract_require (ABSTRACT VirtualFileSystem CONCRETE_INTERFACE Container INTERFACE APICommon)
abstract_register_implementation (NAME Original PARTS ResourceProviderOriginal)
