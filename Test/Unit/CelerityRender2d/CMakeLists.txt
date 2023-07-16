if (NOT EMERGENCE_INCLUDE_GPU_DEPENDANT_TESTS)
    return ()
endif ()

register_concrete (CelerityRender2dTests)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Private")
concrete_sources ("Private/*.cpp" "Resources/Shaders/*.sc")

concrete_require (
        SCOPE PRIVATE

        CONCRETE_INTERFACE
        FileSystemTestUtility CelerityRender2d CelerityTransform ResourceContextHolder SDLContextHolder

        INTERFACE MemoryProfilerStub Testing
        THIRD_PARTY SDL3::SDL3)
register_test_default_resource_usage_scheme ()

register_executable (TestCelerityRender2d)
executable_include (
        ABSTRACT
        Assert=SDL3 CPUProfiler=None Hashing=XXHash JobDispatcher=Original Log=SPDLog Memory=Original
        MemoryProfiler=Original RecordCollection=Pegasus RenderBackend=BGFX ResourceProvider=Original
        StandardLayoutMapping=Original TaskExecutor=Parallel VirtualFileSystem=Original Warehouse=Galleon

        CONCRETE
        Celerity CelerityAsset CelerityRender2d CelerityRender2dTests CelerityRenderFoundation CelerityTransform
        Container FileSystemTestUtility Flow Handling Math RecordCollectionVisualization ResourceContextHolder
        SDLContextHolder Serialization SyntaxSugar TaskCollection VisualGraph)
executable_verify ()
executable_copy_linked_artefacts ()
executable_deploy_resource_mount_lists ("${CMAKE_CURRENT_BINARY_DIR}")

add_test (NAME "TestCelerityRender2d" COMMAND TestCelerityRender2d)
add_dependencies (EmergenceTests TestCelerityRender2d)
