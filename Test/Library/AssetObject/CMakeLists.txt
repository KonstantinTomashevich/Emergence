file (GLOB_RECURSE SOURCES *.cpp)
file (GLOB_RECURSE HEADERS *.hpp)

add_executable (TestAssetObjectOriginal ${SOURCES} ${HEADERS})
add_test (NAME "TestAssetObjectOriginal" COMMAND TestAssetObjectOriginal)
add_dependencies (LibraryTests TestAssetObjectOriginal)
target_include_directories (TestAssetObjectOriginal PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

sober_naming_variant_target (AssetObject Original ASSET_OBJECT_LIBRARY_TARGET)
sober_naming_implementation_target (JobDispatcher Original JOB_DISPATCHER_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (Log SPDLog LOG_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (Memory Original MEMORY_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (MemoryProfiler Original MEMORY_PROFILER_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (StandardLayoutMapping Original STANDARD_LAYOUT_MAPPING_IMPLEMENTATION_TARGET)
sober_naming_variant_target (MemoryProfilerStub Original MEMORY_PROFILER_STUB_TARGET)
sober_naming_implementation_target (Testing Doctest TESTING_IMPLEMENTATION_TARGET)

target_link_libraries (TestAssetObjectOriginal PRIVATE
        ${ASSET_OBJECT_LIBRARY_TARGET} ${JOB_DISPATCHER_IMPLEMENTATION_TARGET}
        ${LOG_IMPLEMENTATION_TARGET} ${MEMORY_IMPLEMENTATION_TARGET}
        ${MEMORY_PROFILER_IMPLEMENTATION_TARGET} ${STANDARD_LAYOUT_MAPPING_IMPLEMENTATION_TARGET}
        ${MEMORY_PROFILER_STUB_TARGET} ${TESTING_IMPLEMENTATION_TARGET})

# On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
if (MSVC)
    target_compile_options (TestAssetObjectOriginal PRIVATE /EHa)
endif ()
