register_concrete (MemoryProfilerNone)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")
concrete_require (SCOPE PRIVATE ABSTRACT MemoryProfiler CONCRETE_INTERFACE SyntaxSugar)
