register_concrete (FlowTests)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")
concrete_require (SCOPE PRIVATE CONCRETE_INTERFACE Flow INTERFACE MemoryProfilerStub Testing)

register_executable (TestFlow)
executable_include (
        ABSTRACT
        Assert=SDL3 CPUProfiler=None Log=SPDLog Memory=Original
        MemoryProfiler=Original StandardLayoutMapping=Original

        CONCRETE Container Flow FlowTests Handling SyntaxSugar TaskCollection VisualGraph)
executable_verify ()
executable_copy_linked_artefacts ()

add_test (NAME "TestFlow" COMMAND TestFlow)
add_dependencies (EmergenceTests TestFlow)
