register_concrete (RecordCollectionPegasus)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")
concrete_require (SCOPE PRIVATE ABSTRACT RecordCollection CONCRETE_INTERFACE Pegasus)
