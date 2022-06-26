file (GLOB_RECURSE SOURCES *.cpp)
file (GLOB_RECURSE HEADERS *.hpp)

add_executable (TestCelerityTransformOriginal ${SOURCES} ${HEADERS})
add_test (NAME "TestCelerityTransformOriginal" COMMAND TestCelerityTransformOriginal)
add_dependencies (LibraryTests TestCelerityTransformOriginal)
target_include_directories (TestCelerityTransformOriginal PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

sober_naming_implementation_target (Log SPDLog LOG_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (Memory Original MEMORY_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (MemoryProfiler Original MEMORY_PROFILER_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (RecordCollection PegasusOriginal RECORD_COLLECTION_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (StandardLayoutMapping Original STANDARD_LAYOUT_MAPPING_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (TaskExecutor Parallel TASK_EXECUTOR_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (Testing Doctest TESTING_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (Warehouse GalleonOriginal WAREHOUSE_IMPLEMENTATION_TARGET)
sober_naming_variant_target (CelerityTransform Original CELERITY_TRANSFORM_LIBRARY_TARGET)
sober_naming_variant_target (MemoryProfilerStub Original MEMORY_PROFILER_STUB_TARGET)

target_link_libraries (TestCelerityTransformOriginal PRIVATE
        ${LOG_IMPLEMENTATION_TARGET} ${MEMORY_IMPLEMENTATION_TARGET}
        ${MEMORY_PROFILER_IMPLEMENTATION_TARGET} ${RECORD_COLLECTION_IMPLEMENTATION_TARGET}
        ${STANDARD_LAYOUT_MAPPING_IMPLEMENTATION_TARGET} ${TASK_EXECUTOR_IMPLEMENTATION_TARGET}
        ${TESTING_IMPLEMENTATION_TARGET} ${WAREHOUSE_IMPLEMENTATION_TARGET}
        ${CELERITY_TRANSFORM_LIBRARY_TARGET} ${MEMORY_PROFILER_STUB_TARGET})

# On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
if (MSVC)
    target_compile_options (TestCelerityTransformOriginal PRIVATE /EHa)
endif ()
