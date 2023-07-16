register_concrete (CelerityAssetTests)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")
concrete_require (SCOPE PRIVATE CONCRETE_INTERFACE CelerityAsset INTERFACE MemoryProfilerStub Testing)

register_executable (TestCelerityAsset)
executable_include (
        ABSTRACT
        Assert=SDL3 CPUProfiler=None Hashing=XXHash JobDispatcher=Original Log=SPDLog Memory=Original
        MemoryProfiler=Original RecordCollection=Pegasus ResourceProvider=Original StandardLayoutMapping=Original
        TaskExecutor=Parallel VirtualFileSystem=Original Warehouse=Galleon

        CONCRETE
        Celerity CelerityAsset CelerityAssetTests Container Flow Handling Math
        RecordCollectionVisualization Serialization SyntaxSugar TaskCollection VisualGraph)
executable_verify ()
executable_copy_linked_artefacts ()

add_test (NAME "TestCelerityAsset" COMMAND TestCelerityAsset)
add_dependencies (EmergenceTests TestCelerityAsset)
