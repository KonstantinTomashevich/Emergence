register_concrete (CelerityAssemblyTests)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")
concrete_require (
        SCOPE PRIVATE
        CONCRETE_INTERFACE CelerityAssembly CelerityTransform
        INTERFACE MemoryProfilerStub Testing)

register_executable (TestCelerityAssembly)
executable_include (
        ABSTRACT
        Assert=SDL3 CPUProfiler=None Hashing=XXHash JobDispatcher=Original Log=SPDLog Memory=Original
        MemoryProfiler=Original RecordCollection=Pegasus ResourceProvider=Original StandardLayoutMapping=Original
        TaskExecutor=Parallel VirtualFileSystem=Original Warehouse=Galleon

        CONCRETE
        Celerity CelerityAssembly CelerityAssemblyTests CelerityTransform Container Flow Handling Math
        RecordCollectionVisualization Serialization SyntaxSugar TaskCollection VisualGraph)
executable_verify ()
executable_copy_linked_artefacts ()

add_test (NAME "TestCelerityAssembly" COMMAND TestCelerityAssembly)
add_dependencies (EmergenceTests TestCelerityAssembly)
