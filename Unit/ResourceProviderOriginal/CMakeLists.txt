register_concrete (ResourceProviderOriginal)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")

concrete_require (
        SCOPE PRIVATE
        ABSTRACT Log RecordCollection
        CONCRETE_INTERFACE Container Serialization)
concrete_implements_abstract (ResourceProvider)
