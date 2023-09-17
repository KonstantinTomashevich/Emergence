register_concrete (CelerityPhysics2dTests)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")

concrete_require (
        SCOPE PRIVATE
        ABSTRACT CelerityPhysics2dLogic
        CONCRETE_INTERFACE CelerityTransformLogic
        INTERFACE MemoryProfilerStub Testing)

abstract_get_implementations (ABSTRACT CelerityPhysics2dLogic OUTPUT IMPLEMENTATIONS)
foreach (IMPLEMENTATION ${IMPLEMENTATIONS})
    register_executable (TestCelerityPhysics2d${IMPLEMENTATION})
    executable_include (
            ABSTRACT
            Assert=SDL3 Box2dAdapter=Profiled CelerityPhysics2dLogic=${IMPLEMENTATION}
            CelerityPhysics2dModel=${IMPLEMENTATION} CPUProfiler=None Hashing=XXHash JobDispatcher=Original Log=SPDLog
            Memory=Original MemoryProfiler=Original RecordCollection=Pegasus ResourceProvider=Original
            StandardLayoutMapping=Original TaskExecutor=Parallel VirtualFileSystem=Original Warehouse=Galleon

            CONCRETE
            Celerity CelerityPhysics2dTests CelerityTransformLogic CelerityTransformModel Container Flow Handling Math
            RecordCollectionVisualization Serialization TaskCollection Threading Time VisualGraph)
    executable_verify ()
    executable_copy_linked_artefacts ()

    add_test (NAME "TestCelerityPhysics2d${IMPLEMENTATION}" COMMAND TestCelerityPhysics2d${IMPLEMENTATION})
    add_dependencies (EmergenceTests TestCelerityPhysics2d${IMPLEMENTATION})
endforeach ()
