register_concrete (ResourceCooking)
concrete_include (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")

concrete_require (
        SCOPE PRIVATE
        ABSTRACT Log
        CONCRETE_INTERFACE Serialization
        INTERFACE ResourceProviderHelpers VirtualFileSystemHelpers)
concrete_require (SCOPE PUBLIC ABSTRACT RecordCollection ResourceProvider CONCRETE_INTERFACE Container)
