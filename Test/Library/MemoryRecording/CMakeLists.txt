file (GLOB_RECURSE SOURCES *.cpp)
file (GLOB_RECURSE HEADERS *.hpp)

add_executable (TestMemoryRecordingOriginal ${SOURCES} ${HEADERS})
add_test (NAME "TestMemoryRecordingOriginal" COMMAND TestMemoryRecordingOriginal)
add_dependencies (LibraryTests TestMemoryRecordingOriginal)
target_include_directories (TestMemoryRecordingOriginal PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

sober_naming_implementation_target (Log SPDLog LOG_IMPLEMENTATION_TARGET)
sober_naming_variant_target (MemoryRecording Original MEMORY_RECORDING_LIBRARY_TARGET)
sober_naming_implementation_target (Memory Original MEMORY_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (MemoryProfiler Original MEMORY_PROFILER_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (StandardLayoutMapping Original STANDARD_LAYOUT_MAPPING_IMPLEMENTATION_TARGET)
sober_naming_variant_target (SyntaxSugar Original SYNTAX_SUGAR_LIBRARY_TARGET)
sober_naming_implementation_target (Testing Doctest TESTING_IMPLEMENTATION_TARGET)

target_link_libraries (
        TestMemoryRecordingOriginal PRIVATE
        ${LOG_IMPLEMENTATION_TARGET} ${MEMORY_RECORDING_LIBRARY_TARGET} ${MEMORY_IMPLEMENTATION_TARGET}
        ${MEMORY_PROFILER_IMPLEMENTATION_TARGET} ${STANDARD_LAYOUT_MAPPING_IMPLEMENTATION_TARGET}
        ${SYNTAX_SUGAR_LIBRARY_TARGET} ${TESTING_IMPLEMENTATION_TARGET})

# On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
if (MSVC)
    target_compile_options (TestMemoryRecordingOriginal PRIVATE /EHa)
endif ()
