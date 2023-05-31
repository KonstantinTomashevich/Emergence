sober_library_begin (ParallelTaskExecutor OBJECT)
    file (GLOB_RECURSE HEADERS "*.hpp")
    file (GLOB_RECURSE SOURCES "*.cpp")
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

    sober_library_use_service (EXPECTED PRIVATE JobDispatcher)

    sober_naming_service_target (TaskExecutor TASK_EXECUTOR_API_TARGET)
    sober_naming_variant_target (APICommon Original API_COMMON_LIBRARY_TARGET)
    sober_naming_variant_target (Container Original CONTAINER_LIBRARY_TARGET)
    sober_naming_variant_target (SyntaxSugar Original SYNTAX_SUGAR_LIBRARY_TARGET)

    sober_library_link_library (PRIVATE "${TASK_EXECUTOR_API_TARGET}")
    sober_library_link_library (PUBLIC "${API_COMMON_LIBRARY_TARGET}")
    sober_library_link_library (PUBLIC "${CONTAINER_LIBRARY_TARGET}")
    sober_library_link_library (PUBLIC "${SYNTAX_SUGAR_LIBRARY_TARGET}")

    sober_variant_begin (Original)
    sober_variant_end ()
sober_library_end ()
