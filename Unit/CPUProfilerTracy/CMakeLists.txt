register_concrete (CPUProfilerTracy)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")
concrete_require (SCOPE PRIVATE ABSTRACT CPUProfiler CONCRETE_INTERFACE SyntaxSugar THIRD_PARTY Tracy::TracyClient)
