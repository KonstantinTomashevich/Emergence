register_concrete (CelerityResourceConfigTests)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")

concrete_require (
        SCOPE PRIVATE
        CONCRETE_INTERFACE CelerityResourceConfig Serialization
        INTERFACE MemoryProfilerStub Testing)

register_executable (TestCelerityResourceConfig)
executable_include (
        ABSTRACT
        Assert=SDL3 CPUProfiler=None Hashing=XXHash JobDispatcher=Original Log=SPDLog Memory=Original
        MemoryProfiler=Original RecordCollection=Pegasus ResourceProvider=Original StandardLayoutMapping=Original
        TaskExecutor=Parallel VirtualFileSystem=Original Warehouse=Galleon

        CONCRETE
        Celerity CelerityResourceConfig CelerityResourceConfigTests Container Flow Handling Math
        RecordCollectionVisualization Serialization SyntaxSugar TaskCollection VisualGraph)
executable_verify ()
executable_copy_linked_artefacts ()

add_test (NAME "TestCelerityResourceConfig" COMMAND TestCelerityResourceConfig)
add_dependencies (EmergenceTests TestCelerityResourceConfig)
