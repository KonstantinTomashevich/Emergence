register_concrete (HandlingTests)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")

concrete_require (
        SCOPE PRIVATE
        CONCRETE_INTERFACE Container Handling ReferenceTest
        INTERFACE ContextExtension MemoryProfilerStub Testing)

register_executable (TestHandling)
executable_include (
        ABSTRACT
        Assert=SDL3 CPUProfiler=None Log=SPDLog Memory=Original
        MemoryProfiler=Original StandardLayoutMapping=Original

        CONCRETE Container Handling HandlingTests ReferenceTest Threading Time)
executable_verify ()
executable_copy_linked_artefacts ()

add_test (NAME "TestHandling" COMMAND TestHandling)
add_dependencies (EmergenceTests TestHandling)
