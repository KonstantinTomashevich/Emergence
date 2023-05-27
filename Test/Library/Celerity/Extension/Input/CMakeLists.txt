file (GLOB_RECURSE SOURCES *.cpp)
file (GLOB_RECURSE HEADERS *.hpp)

add_executable (TestCelerityInputOriginal ${SOURCES} ${HEADERS})
add_test (NAME "TestCelerityInputOriginal" COMMAND TestCelerityInputOriginal)
add_dependencies (LibraryTests TestCelerityInputOriginal)
target_include_directories (TestCelerityInputOriginal PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

sober_naming_variant_target (CelerityInput Original CELERITY_INPUT_LIBRARY_TARGET)
sober_target_link_libraries (TestCelerityInputOriginal PRIVATE ${CELERITY_INPUT_LIBRARY_TARGET})
link_celerity_with_dependencies (TestCelerityInputOriginal)

# On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
if (MSVC)
    target_compile_options (TestCelerityInputOriginal PRIVATE /EHa)
endif ()
