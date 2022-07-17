file (GLOB_RECURSE SOURCES *.cpp)
file (GLOB_RECURSE HEADERS *.hpp)

add_executable (TestSerializationOriginal ${SOURCES} ${HEADERS})
add_test (NAME "TestSerializationOriginal" COMMAND TestSerializationOriginal)
add_dependencies (LibraryTests TestSerializationOriginal)
target_include_directories (TestSerializationOriginal PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

sober_naming_variant_target (Serialization Original SERIALIZATION_LIBRARY_TARGET)
sober_naming_implementation_target (Memory Original MEMORY_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (MemoryProfiler Original MEMORY_PROFILER_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (StandardLayoutMapping Original STANDARD_LAYOUT_MAPPING_IMPLEMENTATION_TARGET)
sober_naming_variant_target (MemoryProfilerStub Original MEMORY_PROFILER_STUB_TARGET)
sober_naming_implementation_target (Testing Doctest TESTING_IMPLEMENTATION_TARGET)

target_link_libraries (TestSerializationOriginal PRIVATE
        ${SERIALIZATION_LIBRARY_TARGET} ${MEMORY_IMPLEMENTATION_TARGET}
        ${MEMORY_PROFILER_IMPLEMENTATION_TARGET} ${STANDARD_LAYOUT_MAPPING_IMPLEMENTATION_TARGET}
        ${MEMORY_PROFILER_STUB_TARGET} ${TESTING_IMPLEMENTATION_TARGET})

# On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
if (MSVC)
    target_compile_options (TestSerializationOriginal PRIVATE /EHa)
endif ()
