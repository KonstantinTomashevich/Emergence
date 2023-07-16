register_concrete (MemoryTests)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")
concrete_require (SCOPE PRIVATE ABSTRACT Memory CONCRETE_INTERFACE Container INTERFACE Testing MemoryProfilerStub)

abstract_get_implementations (ABSTRACT Memory OUTPUT IMPLEMENTATIONS)
foreach (IMPLEMENTATION ${IMPLEMENTATIONS})
    register_executable (TestMemory${IMPLEMENTATION})
    executable_include (
            ABSTRACT
            Assert=SDL3 CPUProfiler=None Log=SPDLog Memory=${IMPLEMENTATION}
            MemoryProfiler=Original StandardLayoutMapping=Original

            CONCRETE Container Handling MemoryTests SyntaxSugar)
    executable_verify ()
    executable_copy_linked_artefacts ()

    add_test (NAME "TestMemory${IMPLEMENTATION}" COMMAND TestMemory${IMPLEMENTATION})
    add_dependencies (EmergenceTests TestMemory${IMPLEMENTATION})
endforeach ()
