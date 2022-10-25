file (GLOB_RECURSE SOURCES *.cpp)
file (GLOB_RECURSE HEADERS *.hpp)

add_executable (TestContainerOriginal ${SOURCES} ${HEADERS})
add_test (NAME "TestContainerOriginal" COMMAND TestContainerOriginal)
add_dependencies (LibraryTests TestContainerOriginal)
target_include_directories (TestContainerOriginal PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

sober_naming_implementation_target (Assert SDL2 ASSERT_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (Log SPDLog LOG_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (Memory Original MEMORY_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (MemoryProfiler Original MEMORY_PROFILER_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (StandardLayoutMapping Original STANDARD_LAYOUT_MAPPING_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (Testing Doctest TESTING_IMPLEMENTATION_TARGET)
sober_naming_variant_target (Container Original CONTAINER_LIBRARY_TARGET)
sober_naming_variant_target (MemoryProfilerStub Original MEMORY_PROFILER_STUB_TARGET)

target_link_libraries (
        TestContainerOriginal PRIVATE
        ${ASSERT_IMPLEMENTATION_TARGET} ${LOG_IMPLEMENTATION_TARGET}
        ${MEMORY_IMPLEMENTATION_TARGET} ${MEMORY_PROFILER_IMPLEMENTATION_TARGET}
        ${STANDARD_LAYOUT_MAPPING_IMPLEMENTATION_TARGET} ${TESTING_IMPLEMENTATION_TARGET}
        ${CONTAINER_LIBRARY_TARGET} ${MEMORY_PROFILER_STUB_TARGET})
copy_required_shared_libraries (TestContainerOriginal)

# On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
if (MSVC)
    target_compile_options (TestContainerOriginal PRIVATE /EHa)
endif ()
