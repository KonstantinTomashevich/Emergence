# We have two sets of tests:
# - Tests for really implemented profiling, that check if it works correctly.
# - Tests for stubbed out profiling, that check if it has expected safe behaviour.

register_concrete (MemoryProfilerImplementedTests)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Implemented")
concrete_sources ("Implemented/*.cpp")

concrete_require (
        SCOPE PRIVATE
        ABSTRACT MemoryProfiler
        CONCRETE_INTERFACE Container
        INTERFACE Testing MemoryProfilerStub)

register_concrete (MemoryProfilerStubTests)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Stub")
concrete_sources ("Stub/*.cpp")
concrete_require (SCOPE PRIVATE ABSTRACT MemoryProfiler INTERFACE Testing)

abstract_get_implementations (ABSTRACT MemoryProfiler OUTPUT IMPLEMENTATIONS)
foreach (IMPLEMENTATION ${IMPLEMENTATIONS})
    register_executable (TestMemoryProfiler${IMPLEMENTATION})

    executable_include (
            ABSTRACT
            Assert=SDL3 CPUProfiler=None Log=SPDLog Memory=Original
            MemoryProfiler=${IMPLEMENTATION} StandardLayoutMapping=Original

            CONCRETE Container Handling Threading Time)

    if (IMPLEMENTATION STREQUAL "None")
        executable_include (CONCRETE MemoryProfilerStubTests)
    else ()
        executable_include (CONCRETE MemoryProfilerImplementedTests)
    endif ()

    executable_verify ()
    executable_copy_linked_artefacts ()

    add_test (NAME "TestMemoryProfiler${IMPLEMENTATION}" COMMAND TestMemoryProfiler${IMPLEMENTATION})
    add_dependencies (EmergenceTests TestMemoryProfiler${IMPLEMENTATION})
endforeach ()
