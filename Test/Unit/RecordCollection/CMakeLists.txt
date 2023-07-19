register_concrete (RecordCollectionTests)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")

concrete_require (
        SCOPE PRIVATE
        ABSTRACT RecordCollection
        CONCRETE_INTERFACE QueryTest RecordCollectionVisualization ReferenceTest
        INTERFACE ContextExtension MemoryProfilerStub Testing)

abstract_get_implementations (ABSTRACT RecordCollection OUTPUT IMPLEMENTATIONS)
foreach (IMPLEMENTATION ${IMPLEMENTATIONS})
    register_executable (TestRecordCollection${IMPLEMENTATION})
    executable_include (
            ABSTRACT
            Assert=SDL3 CPUProfiler=None Hashing=XXHash Log=SPDLog Memory=Original
            MemoryProfiler=Original RecordCollection=${IMPLEMENTATION} StandardLayoutMapping=Original

            CONCRETE
            Container Handling QueryTest RecordCollectionTests RecordCollectionVisualization ReferenceTest Threading
            Time VisualGraph)
    executable_verify ()
    executable_copy_linked_artefacts ()

    add_test (NAME "TestRecordCollection${IMPLEMENTATION}" COMMAND TestRecordCollection${IMPLEMENTATION})
    add_dependencies (EmergenceTests TestRecordCollection${IMPLEMENTATION})
endforeach ()
