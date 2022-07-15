file (GLOB_RECURSE SOURCES *.cpp)
file (GLOB_RECURSE HEADERS *.hpp)

add_executable (TestCelerityAssemblyOriginal ${SOURCES} ${HEADERS})
add_test (NAME "TestCelerityAssemblyOriginal" COMMAND TestCelerityAssemblyOriginal)
add_dependencies (LibraryTests TestCelerityAssemblyOriginal)
target_include_directories (TestCelerityAssemblyOriginal PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

sober_naming_variant_target (CelerityAssembly Original CELERITY_TRANSFORM_LIBRARY_TARGET)

target_link_libraries (TestCelerityAssemblyOriginal PRIVATE ${CELERITY_TRANSFORM_LIBRARY_TARGET})
link_celerity_with_dependencies (TestCelerityAssemblyOriginal)

# On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
if (MSVC)
    target_compile_options (TestCelerityAssemblyOriginal PRIVATE /EHa)
endif ()
