file (GLOB_RECURSE SOURCES *.cpp)
file (GLOB_RECURSE HEADERS *.hpp)

add_executable (TestResourceCookingOriginal ${SOURCES} ${HEADERS})
add_test (NAME "TestResourceCookingOriginal" COMMAND TestResourceCookingOriginal)
add_dependencies (LibraryTests TestResourceCookingOriginal)
target_include_directories (TestResourceCookingOriginal PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

sober_naming_implementation_target (Assert SDL3 ASSERT_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (CPUProfiler None CPU_PROFILER_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (Log SPDLog LOG_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (Memory Original MEMORY_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (MemoryProfiler Original MEMORY_PROFILER_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (RecordCollection PegasusOriginal RECORD_COLLECTION_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (ResourceProvider Original RESOURCE_PROVIDER_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (Serialization Original SERIALIZATION_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (StandardLayoutMapping Original STANDARD_LAYOUT_MAPPING_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (Testing Doctest TESTING_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (VirtualFileSystem Original VIRTUAL_FILE_SYSTEM_IMPLEMENTATION_TARGET)
sober_naming_variant_target (MemoryProfilerStub Original MEMORY_PROFILER_STUB_TARGET)
sober_naming_variant_target (ResourceCooking Original RESOURCE_COOKING_LIBRARY_TARGET)

sober_target_link_libraries (TestResourceCookingOriginal PRIVATE
        ${ASSERT_IMPLEMENTATION_TARGET} ${CPU_PROFILER_IMPLEMENTATION_TARGET}
        ${LOG_IMPLEMENTATION_TARGET} ${MEMORY_IMPLEMENTATION_TARGET}
        ${MEMORY_PROFILER_IMPLEMENTATION_TARGET} ${RECORD_COLLECTION_IMPLEMENTATION_TARGET}
        ${RESOURCE_PROVIDER_IMPLEMENTATION_TARGET} ${SERIALIZATION_IMPLEMENTATION_TARGET}
        ${STANDARD_LAYOUT_MAPPING_IMPLEMENTATION_TARGET} ${TESTING_IMPLEMENTATION_TARGET}
        ${VIRTUAL_FILE_SYSTEM_IMPLEMENTATION_TARGET} ${MEMORY_PROFILER_STUB_TARGET} ${RESOURCE_COOKING_LIBRARY_TARGET})
copy_required_shared_libraries (TestResourceCookingOriginal)

# On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
if (MSVC)
    target_compile_options (TestResourceCookingOriginal PRIVATE /EHa)
endif ()
