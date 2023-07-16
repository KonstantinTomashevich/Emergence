add_common_compile_options ()
add_custom_target (EmergenceTests COMMENT "Build all Emergence tests.")
add_subdirectory (Shared)
add_subdirectory (Unit)
