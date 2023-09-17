register_concrete (LogTestsShared)
concrete_include (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Shared")
concrete_sources ("Shared/*.cpp")
concrete_require (SCOPE PUBLIC ABSTRACT Log CONCRETE_INTERFACE Container)

register_concrete (LogTestsDriver)
concrete_include (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Driver")
concrete_sources ("Driver/*.cpp")
concrete_require (SCOPE PUBLIC CONCRETE_INTERFACE LogTestsShared INTERFACE MemoryProfilerStub)

register_executable (TestLogDriver)
executable_include (
        ABSTRACT
        Assert=SDL3 CPUProfiler=None
        # Unfortunately, we need some Log implementation for units that are being used by driver internally.
        Log=SPDLog
        Memory=Original MemoryProfiler=Original StandardLayoutMapping=Original

        CONCRETE Container Handling LogTestsDriver LogTestsShared Threading Time)
executable_verify ()
executable_copy_linked_artefacts ()
add_dependencies (EmergenceTests TestLogDriver)

register_concrete (LogTestsApp)
concrete_include (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/App")
concrete_sources ("App/*.cpp")
concrete_require (SCOPE PUBLIC CONCRETE_INTERFACE LogTestsShared INTERFACE MemoryProfilerStub)

abstract_get_implementations (ABSTRACT Log OUTPUT IMPLEMENTATIONS)
foreach (IMPLEMENTATION ${IMPLEMENTATIONS})
    register_executable (TestLogApp${IMPLEMENTATION})
    executable_include (
            ABSTRACT
            Assert=SDL3 CPUProfiler=None Log=${IMPLEMENTATION} Memory=Original
            MemoryProfiler=Original StandardLayoutMapping=Original

            CONCRETE Container Handling LogTestsApp LogTestsShared Threading Time)
    executable_verify ()
    executable_copy_linked_artefacts ()

    add_test (NAME "TestLogApp${IMPLEMENTATION}" COMMAND TestLogDriver $<TARGET_FILE:TestLogApp${IMPLEMENTATION}>)
    add_dependencies (EmergenceTests TestLogApp${IMPLEMENTATION})
endforeach ()
