file (GLOB_RECURSE SOURCES *.cpp)
file (GLOB_RECURSE HEADERS *.hpp)

add_executable (TestCelerityResourceConfigOriginal ${SOURCES} ${HEADERS})
add_test (NAME "TestCelerityResourceConfigOriginal" COMMAND TestCelerityResourceConfigOriginal)
add_dependencies (LibraryTests TestCelerityResourceConfigOriginal)
target_include_directories (TestCelerityResourceConfigOriginal PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

sober_naming_variant_target (CelerityResourceConfig Original CELERITY_RESOURCE_CONFIG_LIBRARY_TARGET)

target_link_libraries (TestCelerityResourceConfigOriginal PRIVATE ${CELERITY_RESOURCE_CONFIG_LIBRARY_TARGET})
link_celerity_with_dependencies (TestCelerityResourceConfigOriginal)

# On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
if (MSVC)
    target_compile_options (TestCelerityResourceConfigOriginal PRIVATE /EHa)
endif ()
