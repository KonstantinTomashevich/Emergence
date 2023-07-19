register_concrete (CelerityTests)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")
concrete_require (SCOPE PRIVATE CONCRETE_INTERFACE Celerity INTERFACE MemoryProfilerStub Testing)

register_executable (TestCelerity)
executable_include (
        ABSTRACT
        Assert=SDL3 CPUProfiler=None Hashing=XXHash JobDispatcher=Original Log=SPDLog Memory=Original
        MemoryProfiler=Original RecordCollection=Pegasus ResourceProvider=Original StandardLayoutMapping=Original
        TaskExecutor=Parallel VirtualFileSystem=Original Warehouse=Galleon

        CONCRETE
        Celerity CelerityTests Container Flow Handling Math RecordCollectionVisualization
        Serialization TaskCollection Threading Time VisualGraph)
executable_verify ()
executable_copy_linked_artefacts ()

add_test (NAME "TestCelerity" COMMAND TestCelerity)
add_dependencies (EmergenceTests TestCelerity)
