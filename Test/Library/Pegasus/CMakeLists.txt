file (GLOB_RECURSE SOURCES *.cpp)
file (GLOB_RECURSE HEADERS *.hpp)

# We test only Pegasus Original variant, because only internal constants are different.
add_executable (TestPegasusOriginal ${SOURCES} ${HEADERS})
add_test (NAME "TestPegasusOriginal" COMMAND TestPegasusOriginal)
add_dependencies (LibraryTests TestPegasusOriginal)
target_include_directories (TestPegasusOriginal PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

sober_naming_variant_target (Pegasus Original PEGASUS_LIBRARY_TARGET)
sober_naming_variant_target (TestQueryAPI Original TEST_QUERY_API_LIBRARY_TARGET)
target_link_libraries (TestPegasusOriginal PRIVATE ${PEGASUS_LIBRARY_TARGET} ${TEST_QUERY_API_LIBRARY_TARGET})

# On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
if (MSVC)
    target_compile_options (TestPegasusOriginal PRIVATE /EHa)
endif ()
