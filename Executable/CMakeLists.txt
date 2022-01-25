add_common_compile_options ()
add_subdirectory (MemoryRecordingClient)

if (EMERGENCE_SAMPLES)
    add_subdirectory (TopDownShooter)
endif ()
