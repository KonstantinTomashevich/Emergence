register_concrete (TaskExecutorTests)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")
concrete_require (SCOPE PRIVATE ABSTRACT TaskExecutor INTERFACE MemoryProfilerStub Testing)

abstract_get_implementations (ABSTRACT TaskExecutor OUTPUT IMPLEMENTATIONS)
foreach (IMPLEMENTATION ${IMPLEMENTATIONS})
    register_executable (TestTaskExecutor${IMPLEMENTATION})
    executable_include (
            ABSTRACT
            Assert=SDL3 CPUProfiler=None JobDispatcher=Original Log=SPDLog Memory=Original
            MemoryProfiler=Original StandardLayoutMapping=Original TaskExecutor=${IMPLEMENTATION}

            CONCRETE Container Handling TaskCollection TaskExecutorTests Threading Time)
    executable_verify ()
    executable_copy_linked_artefacts ()

    add_test (NAME "TestTaskExecutor${IMPLEMENTATION}" COMMAND TestTaskExecutor${IMPLEMENTATION})
    add_dependencies (EmergenceTests TestTaskExecutor${IMPLEMENTATION})
endforeach ()
