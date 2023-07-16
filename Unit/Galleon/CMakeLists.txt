register_concrete (Galleon)
concrete_include (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*cpp")

concrete_require (
        SCOPE PUBLIC
        ABSTRACT Memory MemoryProfiler RecordCollection
        CONCRETE_INTERFACE Container Handling SyntaxSugar)
