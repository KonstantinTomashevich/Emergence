register_abstract (ExportGraphTestsExpectation)
abstract_include ("${CMAKE_CURRENT_SOURCE_DIR}/Expectation")
abstract_require (CONCRETE_INTERFACE Container)

abstract_get_implementations (ABSTRACT ExportGraph OUTPUT IMPLEMENTATIONS)
foreach (IMPLEMENTATION ${IMPLEMENTATIONS})
    abstract_register_implementation (NAME ${IMPLEMENTATION} PARTS ExportGraphTestsExpectation${IMPLEMENTATION})
endforeach ()

foreach (IMPLEMENTATION ${IMPLEMENTATIONS})
    register_concrete (ExportGraphTestsExpectation${IMPLEMENTATION})
    concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Expectation${IMPLEMENTATION}")
    concrete_sources ("Expectation${IMPLEMENTATION}/*.cpp")
    concrete_require (SCOPE PRIVATE ABSTRACT ExportGraphTestsExpectation)
endforeach ()

register_concrete (ExportGraphTests)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Private")
concrete_sources ("Private/*.cpp")
concrete_require (SCOPE PRIVATE ABSTRACT ExportGraph ExportGraphTestsExpectation INTERFACE MemoryProfilerStub Testing)

foreach (IMPLEMENTATION ${IMPLEMENTATIONS})
    register_executable (TestExportGraph${IMPLEMENTATION})
    executable_include (
            ABSTRACT
            Assert=SDL3 CPUProfiler=None ExportGraph=${IMPLEMENTATION} ExportGraphTestsExpectation=${IMPLEMENTATION}
            Log=SPDLog Memory=Original MemoryProfiler=Original StandardLayoutMapping=Original

            CONCRETE Container ExportGraphTests Handling SyntaxSugar VisualGraph)
    executable_verify ()
    executable_copy_linked_artefacts ()

    add_test (NAME "TestExportGraph${IMPLEMENTATION}" COMMAND TestExportGraph${IMPLEMENTATION})
    add_dependencies (EmergenceTests TestExportGraph${IMPLEMENTATION})
endforeach ()
