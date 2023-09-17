register_abstract (VirtualFileSystem)
abstract_include ("${CMAKE_CURRENT_SOURCE_DIR}")
abstract_require (CONCRETE_INTERFACE Container INTERFACE APICommon)
abstract_register_implementation (NAME Original PARTS VirtualFileSystemOriginal)
