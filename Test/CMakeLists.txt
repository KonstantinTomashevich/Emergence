add_common_compile_options ()
add_custom_target (LibraryTests COMMENT "Build tests for all libraries.")
add_custom_target (ServiceTests COMMENT "Build tests for all services.")
add_custom_target (EmergenceTests COMMENT "Build all Emergence tests." DEPENDS LibraryTests ServiceTests)

add_subdirectory (API)
add_subdirectory (Library)
add_subdirectory (Service)
add_subdirectory (Shared)
