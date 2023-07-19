register_concrete (MemoryRecordingTests)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")
concrete_require (SCOPE PRIVATE CONCRETE_INTERFACE MemoryRecording INTERFACE MemoryProfilerStub Testing Time)

register_executable (TestMemoryRecording)
executable_include (
        ABSTRACT
        Assert=SDL3 CPUProfiler=None Log=SPDLog Memory=Original
        MemoryProfiler=Original StandardLayoutMapping=Original

        CONCRETE Container Handling MemoryRecording MemoryRecordingTests Serialization Threading Time)
executable_verify ()
executable_copy_linked_artefacts ()

add_test (NAME "TestMemoryRecording" COMMAND TestMemoryRecording)
add_dependencies (EmergenceTests TestMemoryRecording)
