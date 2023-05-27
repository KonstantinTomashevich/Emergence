file (GLOB_RECURSE SOURCES *.cpp)
file (GLOB_RECURSE HEADERS *.hpp)

add_executable (TestCelerityTransformOriginal ${SOURCES} ${HEADERS})
add_test (NAME "TestCelerityTransformOriginal" COMMAND TestCelerityTransformOriginal)
add_dependencies (LibraryTests TestCelerityTransformOriginal)
target_include_directories (TestCelerityTransformOriginal PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

sober_naming_variant_target (CelerityTransform Original CELERITY_TRANSFORM_LIBRARY_TARGET)

sober_target_link_libraries (TestCelerityTransformOriginal PRIVATE ${CELERITY_TRANSFORM_LIBRARY_TARGET})
link_celerity_with_dependencies (TestCelerityTransformOriginal)

# On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
if (MSVC)
    target_compile_options (TestCelerityTransformOriginal PRIVATE /EHa)
endif ()
