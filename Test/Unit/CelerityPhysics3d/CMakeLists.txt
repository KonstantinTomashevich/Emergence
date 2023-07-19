# PhysX is the only implementation now, so we cannot declare the abstract unit if it is not found.
if (NOT TARGET PhysX)
    return ()
endif ()

register_concrete (CelerityPhysics3dTests)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")

concrete_require (
        SCOPE PRIVATE
        ABSTRACT CelerityPhysics3d
        CONCRETE_INTERFACE CelerityTransform
        INTERFACE MemoryProfilerStub Testing)

abstract_get_implementations (ABSTRACT CelerityPhysics3d OUTPUT IMPLEMENTATIONS)
foreach (IMPLEMENTATION ${IMPLEMENTATIONS})
    register_executable (TestCelerityPhysics3d${IMPLEMENTATION})
    executable_include (
            ABSTRACT
            Assert=SDL3 CelerityPhysics3d=${IMPLEMENTATION} CPUProfiler=None Hashing=XXHash JobDispatcher=Original
            Log=SPDLog Memory=Original MemoryProfiler=Original RecordCollection=Pegasus ResourceProvider=Original
            StandardLayoutMapping=Original TaskExecutor=Parallel VirtualFileSystem=Original Warehouse=Galleon

            CONCRETE
            Celerity CelerityPhysics3dTests CelerityTransform Container Flow Handling Math
            RecordCollectionVisualization Serialization TaskCollection Threading Time VisualGraph)
    executable_verify ()
    executable_copy_linked_artefacts ()

    add_test (NAME "TestCelerityPhysics3d${IMPLEMENTATION}" COMMAND TestCelerityPhysics3d${IMPLEMENTATION})
    add_dependencies (EmergenceTests TestCelerityPhysics3d${IMPLEMENTATION})
endforeach ()
