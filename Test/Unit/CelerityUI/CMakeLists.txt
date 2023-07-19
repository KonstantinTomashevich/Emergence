if (NOT EMERGENCE_INCLUDE_GPU_DEPENDANT_TESTS)
    return ()
endif ()

register_abstract (CelerityUITestsImplementationStrings)
abstract_include ("${CMAKE_CURRENT_SOURCE_DIR}/ImplementationStrings")
abstract_require (CONCRETE_INTERFACE Container)

abstract_get_implementations (ABSTRACT CelerityUI OUTPUT IMPLEMENTATIONS)
foreach (IMPLEMENTATION ${IMPLEMENTATIONS})
    abstract_register_implementation (NAME ${IMPLEMENTATION} PARTS CelerityUITestsImplementationStrings${IMPLEMENTATION})
endforeach ()

foreach (IMPLEMENTATION ${IMPLEMENTATIONS})
    register_concrete (CelerityUITestsImplementationStrings${IMPLEMENTATION})
    concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/ImplementationStrings${IMPLEMENTATION}")
    concrete_sources ("ImplementationStrings${IMPLEMENTATION}/*.cpp")
    concrete_require (SCOPE PRIVATE ABSTRACT CelerityUITestsImplementationStrings)
endforeach ()

register_concrete (CelerityUITests)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Private")
concrete_sources ("Private/*.cpp")

concrete_require (
        SCOPE PRIVATE
        ABSTRACT CelerityUI CelerityUITestsImplementationStrings
        CONCRETE_INTERFACE CelerityLocale CelerityTransform FileSystemTestUtility ResourceContextHolder SDLContextHolder
        INTERFACE MemoryProfilerStub Testing
        THIRD_PARTY SDL3::SDL3)
register_test_default_resource_usage_scheme ()

foreach (IMPLEMENTATION ${IMPLEMENTATIONS})
    register_executable (TestCelerityUI${IMPLEMENTATION})
    executable_include (
            ABSTRACT
            Assert=SDL3 CelerityUI=${IMPLEMENTATION} CelerityUITestsImplementationStrings=${IMPLEMENTATION}
            CPUProfiler=None Hashing=XXHash JobDispatcher=Original Log=SPDLog Memory=Original MemoryProfiler=Original
            RecordCollection=Pegasus RenderBackend=BGFX ResourceProvider=Original StandardLayoutMapping=Original
            TaskExecutor=Parallel VirtualFileSystem=Original Warehouse=Galleon

            CONCRETE
            Celerity CelerityAsset CelerityInput CelerityLocale CelerityRenderFoundation CelerityUITests
            CelerityTransform Container FileSystemTestUtility Flow Handling Math RecordCollectionVisualization
            ResourceContextHolder SDLContextHolder Serialization TaskCollection Threading Time VisualGraph)
    executable_verify ()
    executable_copy_linked_artefacts ()
    executable_deploy_resource_mount_lists ("${CMAKE_CURRENT_BINARY_DIR}")

    add_test (NAME "TestCelerityUI${IMPLEMENTATION}" COMMAND TestCelerityUI${IMPLEMENTATION})
    add_dependencies (EmergenceTests TestCelerityUI${IMPLEMENTATION})
endforeach ()
