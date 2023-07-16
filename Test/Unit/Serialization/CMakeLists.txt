register_concrete (SerializationTests)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")
concrete_require (SCOPE PRIVATE CONCRETE_INTERFACE Serialization INTERFACE MemoryProfilerStub Testing)

register_executable (TestSerialization)
executable_include (
        ABSTRACT
        Assert=SDL3 CPUProfiler=None Log=SPDLog Memory=Original
        MemoryProfiler=Original StandardLayoutMapping=Original

        CONCRETE Container Handling Serialization SerializationTests SyntaxSugar)
executable_verify ()
executable_copy_linked_artefacts ()

add_test (NAME "TestSerialization" COMMAND TestSerialization)
add_dependencies (EmergenceTests TestSerialization)
