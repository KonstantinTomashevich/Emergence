register_concrete (HashingTests)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")
concrete_require (SCOPE PRIVATE ABSTRACT Hashing CONCRETE_INTERFACE Container INTERFACE MemoryProfilerStub Testing)

abstract_get_implementations (ABSTRACT Hashing OUTPUT IMPLEMENTATIONS)
foreach (IMPLEMENTATION ${IMPLEMENTATIONS})
    register_executable (TestHashing${IMPLEMENTATION})
    executable_include (
            ABSTRACT
            Assert=SDL3 CPUProfiler=None Hashing=${IMPLEMENTATION} Log=SPDLog Memory=Original
            MemoryProfiler=Original StandardLayoutMapping=Original

            CONCRETE Container Handling HashingTests SyntaxSugar VisualGraph)
    executable_verify ()
    executable_copy_linked_artefacts ()

    add_test (NAME "TestHashing${IMPLEMENTATION}" COMMAND TestHashing${IMPLEMENTATION})
    add_dependencies (EmergenceTests TestHashing${IMPLEMENTATION})
endforeach ()
