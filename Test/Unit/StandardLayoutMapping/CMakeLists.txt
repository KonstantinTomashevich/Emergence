register_concrete (StandardLayoutMappingTests)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")

concrete_require (
        SCOPE PRIVATE
        CONCRETE_INTERFACE Container StandardLayoutMappingVisualization VisualGraph
        INTERFACE MemoryProfilerStub Testing)

abstract_get_implementations (ABSTRACT StandardLayoutMapping OUTPUT IMPLEMENTATIONS)
foreach (IMPLEMENTATION ${IMPLEMENTATIONS})
    register_executable (TestStandardLayoutMapping${IMPLEMENTATION})
    executable_include (
            ABSTRACT
            Assert=SDL3 CPUProfiler=None Log=SPDLog Memory=Original
            MemoryProfiler=Original StandardLayoutMapping=${IMPLEMENTATION}

            CONCRETE
            Container Handling StandardLayoutMappingTests StandardLayoutMappingVisualization Threading Time VisualGraph)
    executable_verify ()
    executable_copy_linked_artefacts ()

    add_test (NAME "TestStandardLayoutMapping${IMPLEMENTATION}" COMMAND TestStandardLayoutMapping${IMPLEMENTATION})
    add_dependencies (EmergenceTests TestStandardLayoutMapping${IMPLEMENTATION})
endforeach ()
