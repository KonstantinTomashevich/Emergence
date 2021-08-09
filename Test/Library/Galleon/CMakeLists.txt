file (GLOB_RECURSE SOURCES *.cpp)
file (GLOB_RECURSE HEADERS *.hpp)

add_executable (TestGalleonOriginal ${SOURCES} ${HEADERS})
add_test (NAME "TestGalleonOriginal" COMMAND TestGalleonOriginal)
add_dependencies (LibraryTests TestGalleonOriginal)
target_include_directories (TestGalleonOriginal PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

sober_naming_variant_target (Galleon Original GALLEON_LIBRARY_TARGET)
sober_naming_variant_target (TestQueryAPI Original TEST_QUERY_API_LIBRARY_TARGET)
sober_naming_variant_target (TestReferenceAPI Original TEST_REFERENCE_API_LIBRARY_TARGET)

target_link_libraries (
        TestGalleonOriginal PRIVATE
        ${GALLEON_LIBRARY_TARGET} ${TEST_QUERY_API_LIBRARY_TARGET} ${TEST_REFERENCE_API_LIBRARY_TARGET})

if (MSVC)
    # On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
    target_compile_options (TestGalleonOriginal PRIVATE /EHa)

    # Scenario object file is too big for MSVC default configuration, therefore we need to increase object size.
    target_compile_options (TestGalleonOriginal PRIVATE /bigobj)
endif ()
