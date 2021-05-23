file (GLOB_RECURSE SOURCES *.cpp)
file (GLOB_RECURSE HEADERS *.hpp)

add_executable (TestHandlingOriginal ${SOURCES} ${HEADERS})
add_test (NAME "TestHandlingOriginal" COMMAND TestHandlingOriginal)
target_include_directories (TestHandlingOriginal PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

sober_naming_variant_target (Handling Original HANDLING_LIBRARY_TARGET)
target_link_libraries (TestHandlingOriginal PRIVATE Boost::unit_test_framework ${HANDLING_LIBRARY_TARGET})

# On MSVC CLang exceptions are disabled by default, but Boost needs them for data driven test cases.
if (MSVC)
    target_compile_options (TestHandlingOriginal PRIVATE /EHa)
endif ()