register_concrete (ResourceProviderTests)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")

concrete_require (
        SCOPE PRIVATE
        ABSTRACT ResourceProvider
        CONCRETE_INTERFACE Serialization
        INTERFACE MemoryProfilerStub Testing)

abstract_get_implementations (ABSTRACT ResourceProvider OUTPUT IMPLEMENTATIONS)
foreach (IMPLEMENTATION ${IMPLEMENTATIONS})
    register_executable (TestResourceProvider${IMPLEMENTATION})
    executable_include (
            ABSTRACT
            Assert=SDL3 CPUProfiler=None Hashing=XXHash Log=SPDLog Memory=Original MemoryProfiler=Original
            RecordCollection=Pegasus ResourceProvider=${IMPLEMENTATION} StandardLayoutMapping=Original
            VirtualFileSystem=Original

            CONCRETE Container Handling ResourceProviderTests Serialization SyntaxSugar)
    executable_verify ()
    executable_copy_linked_artefacts ()

    add_test (NAME "TestResourceProvider${IMPLEMENTATION}" COMMAND TestResourceProvider${IMPLEMENTATION})
    add_dependencies (EmergenceTests TestResourceProvider${IMPLEMENTATION})
endforeach ()
