file (GLOB_RECURSE SOURCES *.cpp)
file (GLOB_RECURSE HEADERS *.hpp)

add_executable (TestHandlingOriginal ${SOURCES} ${HEADERS})
add_test (NAME "TestHandlingOriginal" COMMAND TestHandlingOriginal)
add_dependencies (LibraryTests TestHandlingOriginal)
target_include_directories (TestHandlingOriginal PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

sober_naming_variant_target (ContextExtension Original CONTEXT_EXTENSION_LIBRARY_TARGET)
sober_naming_variant_target (Handling Original HANDLING_LIBRARY_TARGET)
sober_naming_implementation_target (Memory Original MEMORY_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (MemoryProfiler Original MEMORY_PROFILER_IMPLEMENTATION_TARGET)
sober_naming_variant_target (TestReferenceAPI Original TEST_REFERENCE_API_LIBRARY_TARGET)
sober_naming_implementation_target (Testing Doctest TESTING_IMPLEMENTATION_TARGET)

target_link_libraries (
        TestHandlingOriginal PRIVATE
        ${CONTEXT_EXTENSION_LIBRARY_TARGET} ${HANDLING_LIBRARY_TARGET}
        ${MEMORY_IMPLEMENTATION_TARGET} ${MEMORY_PROFILER_IMPLEMENTATION_TARGET}
        ${TEST_REFERENCE_API_LIBRARY_TARGET} ${TESTING_IMPLEMENTATION_TARGET})

# On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
if (MSVC)
    target_compile_options (TestHandlingOriginal PRIVATE /EHa)
endif ()
