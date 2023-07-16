register_concrete (ResourceObjectTests)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")
concrete_require (SCOPE PRIVATE CONCRETE_INTERFACE ResourceObject Serialization INTERFACE MemoryProfilerStub Testing)

register_executable (TestResourceObject)
executable_include (
        ABSTRACT
        Assert=SDL3 CPUProfiler=None Hashing=XXHash Log=SPDLog Memory=Original MemoryProfiler=Original
        RecordCollection=Pegasus ResourceProvider=Original StandardLayoutMapping=Original VirtualFileSystem=Original

        CONCRETE Container Handling ResourceObject ResourceObjectTests Serialization SyntaxSugar)
executable_verify ()
executable_copy_linked_artefacts ()

add_test (NAME "TestResourceObject" COMMAND TestResourceObject)
add_dependencies (EmergenceTests TestResourceObject)
