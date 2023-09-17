register_concrete (ResourceContextHolder)
concrete_include (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")
concrete_require (SCOPE PUBLIC ABSTRACT ResourceProvider)

concrete_require (
        SCOPE PRIVATE
        CONCRETE_INTERFACE Serialization
        INTERFACE ResourceProviderHelpers Testing VirtualFileSystemHelpers)
