register_concrete (MemoryRecordingClientApplication)
concrete_include (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")
concrete_require (SCOPE PRIVATE ABSTRACT Log CONCRETE_INTERFACE MemoryRecording THIRD_PARTY SDL3::SDL3 SDL3ImGUI)

register_executable (MemoryRecordingClient)
executable_include (
        ABSTRACT
        Assert=SDL3 CPUProfiler=None Log=SPDLog Memory=Original
        MemoryProfiler=Original StandardLayoutMapping=Original

        CONCRETE Container Handling MemoryRecording MemoryRecordingClientApplication Serialization SyntaxSugar)
executable_verify ()
executable_copy_linked_artefacts ()

set_target_properties (MemoryRecordingClient PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Tools/MemoryRecordingClient")
