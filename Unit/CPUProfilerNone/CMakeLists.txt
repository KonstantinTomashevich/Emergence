register_concrete (CPUProfilerNone)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")
concrete_implements_abstract (CPUProfiler)
