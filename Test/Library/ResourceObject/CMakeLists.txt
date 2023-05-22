file (GLOB_RECURSE SOURCES *.cpp)
file (GLOB_RECURSE HEADERS *.hpp)

add_executable (TestResourceObjectOriginal ${SOURCES} ${HEADERS})
add_test (NAME "TestResourceObjectOriginal" COMMAND TestResourceObjectOriginal)
add_dependencies (LibraryTests TestResourceObjectOriginal)
target_include_directories (TestResourceObjectOriginal PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

sober_naming_implementation_target (Assert SDL2 ASSERT_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (JobDispatcher Original JOB_DISPATCHER_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (Log SPDLog LOG_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (Memory Original MEMORY_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (MemoryProfiler Original MEMORY_PROFILER_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (RecordCollection PegasusOriginal RECORD_COLLECTION_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (ResourceProvider Original RESOURCE_PROVIDER_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (Serialization Original SERIALIZATION_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (StandardLayoutMapping Original STANDARD_LAYOUT_MAPPING_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (Testing Doctest TESTING_IMPLEMENTATION_TARGET)
sober_naming_variant_target (MemoryProfilerStub Original MEMORY_PROFILER_STUB_TARGET)
sober_naming_variant_target (ResourceObject Original RESOURCE_OBJECT_LIBRARY_TARGET)

target_link_libraries (TestResourceObjectOriginal PRIVATE
        ${ASSERT_IMPLEMENTATION_TARGET} ${JOB_DISPATCHER_IMPLEMENTATION_TARGET}
        ${LOG_IMPLEMENTATION_TARGET} ${MEMORY_IMPLEMENTATION_TARGET}
        ${MEMORY_PROFILER_IMPLEMENTATION_TARGET} ${RECORD_COLLECTION_IMPLEMENTATION_TARGET}
        ${RESOURCE_PROVIDER_IMPLEMENTATION_TARGET} ${SERIALIZATION_IMPLEMENTATION_TARGET}
        ${STANDARD_LAYOUT_MAPPING_IMPLEMENTATION_TARGET} ${TESTING_IMPLEMENTATION_TARGET}
        ${MEMORY_PROFILER_STUB_TARGET} ${RESOURCE_OBJECT_LIBRARY_TARGET})
copy_required_shared_libraries (TestResourceObjectOriginal)

# On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
if (MSVC)
    target_compile_options (TestResourceObjectOriginal PRIVATE /EHa)
endif ()
