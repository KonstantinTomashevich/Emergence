register_concrete (ContainerTests)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")
concrete_require (SCOPE PRIVATE CONCRETE_INTERFACE Container INTERFACE MemoryProfilerStub Testing)

register_executable (TestContainer)
executable_include (
        ABSTRACT
        Assert=SDL3 CPUProfiler=None Log=SPDLog Memory=Original
        MemoryProfiler=Original StandardLayoutMapping=Original

        CONCRETE Container ContainerTests Handling Threading Time)
executable_verify ()
executable_copy_linked_artefacts ()

add_test (NAME "TestContainer" COMMAND TestContainer)
add_dependencies (EmergenceTests TestContainer)
