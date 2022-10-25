function (link_celerity_with_dependencies TARGET_NAME)
    sober_naming_implementation_target (Assert SDL2 ASSERT_IMPLEMENTATION_TARGET)
    sober_naming_implementation_target (JobDispatcher Original JOB_DISPATCHER_IMPLEMENTATION_TARGET)
    sober_naming_implementation_target (Log SPDLog LOG_IMPLEMENTATION_TARGET)
    sober_naming_implementation_target (Memory Original MEMORY_IMPLEMENTATION_TARGET)
    sober_naming_implementation_target (MemoryProfiler Original MEMORY_PROFILER_IMPLEMENTATION_TARGET)
    sober_naming_implementation_target (RecordCollection PegasusOriginal RECORD_COLLECTION_IMPLEMENTATION_TARGET)
    sober_naming_implementation_target (StandardLayoutMapping Original STANDARD_LAYOUT_MAPPING_IMPLEMENTATION_TARGET)
    sober_naming_implementation_target (TaskExecutor Parallel TASK_EXECUTOR_IMPLEMENTATION_TARGET)
    sober_naming_implementation_target (Testing Doctest TESTING_IMPLEMENTATION_TARGET)
    sober_naming_implementation_target (Warehouse GalleonOriginal WAREHOUSE_IMPLEMENTATION_TARGET)
    sober_naming_variant_target (Celerity Original CELERITY_LIBRARY_TARGET)
    sober_naming_variant_target (MemoryProfilerStub Original MEMORY_PROFILER_STUB_TARGET)

    target_link_libraries ("${TARGET_NAME}" PRIVATE
            ${ASSERT_IMPLEMENTATION_TARGET} ${JOB_DISPATCHER_IMPLEMENTATION_TARGET}
            ${LOG_IMPLEMENTATION_TARGET} ${MEMORY_IMPLEMENTATION_TARGET}
            ${MEMORY_PROFILER_IMPLEMENTATION_TARGET} ${RECORD_COLLECTION_IMPLEMENTATION_TARGET}
            ${STANDARD_LAYOUT_MAPPING_IMPLEMENTATION_TARGET} ${TASK_EXECUTOR_IMPLEMENTATION_TARGET}
            ${TESTING_IMPLEMENTATION_TARGET} ${WAREHOUSE_IMPLEMENTATION_TARGET}
            ${CELERITY_LIBRARY_TARGET} ${MEMORY_PROFILER_STUB_TARGET})
    copy_required_shared_libraries (${TARGET_NAME})
endfunction ()

add_subdirectory (Core)
add_subdirectory (Extension)
