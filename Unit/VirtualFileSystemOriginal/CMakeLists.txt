register_concrete (VirtualFileSystemOriginal)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")

concrete_require (
        SCOPE PRIVATE
        ABSTRACT Log RecordCollection VirtualFileSystem
        CONCRETE_INTERFACE Container Serialization SyntaxSugar)
