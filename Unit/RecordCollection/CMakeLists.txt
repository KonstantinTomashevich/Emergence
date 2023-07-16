register_abstract (RecordCollection)
abstract_include ("${CMAKE_CURRENT_SOURCE_DIR}")
abstract_require (ABSTRACT StandardLayoutMapping INTERFACE APICommon)
abstract_register_implementation (NAME Pegasus PARTS Pegasus RecordCollectionPegasus)
