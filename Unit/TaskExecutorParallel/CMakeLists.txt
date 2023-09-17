register_concrete (TaskExecutorParallel)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")

concrete_require (
        SCOPE PRIVATE
        ABSTRACT CPUProfiler JobDispatcher
        CONCRETE_INTERFACE Container Threading
        INTERFACE APICommon)
concrete_implements_abstract (TaskExecutor)
