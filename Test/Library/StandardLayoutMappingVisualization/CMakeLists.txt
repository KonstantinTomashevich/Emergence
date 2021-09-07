file (GLOB_RECURSE SOURCES *.cpp)
file (GLOB_RECURSE HEADERS *.hpp)

add_executable (TestStandardLayoutMappingVisualizationOriginal ${SOURCES} ${HEADERS})
add_test (NAME "TestStandardLayoutMappingVisualizationOriginal" COMMAND TestStandardLayoutMappingVisualizationOriginal)
add_dependencies (LibraryTests TestStandardLayoutMappingVisualizationOriginal)
target_include_directories (TestStandardLayoutMappingVisualizationOriginal PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

sober_naming_variant_target (StandardLayoutMappingVisualization Original VISUALIZATION_LIBRARY_TARGET)
sober_naming_variant_target (SyntaxSugar Original SYNTAX_SUGAR_LIBRARY_TARGET)
sober_naming_variant_target (Testing Doctest TESTING_LIBRARY_TARGET)

target_link_libraries (
        TestStandardLayoutMappingVisualizationOriginal PRIVATE
        ${VISUALIZATION_LIBRARY_TARGET} ${SYNTAX_SUGAR_LIBRARY_TARGET} ${TESTING_LIBRARY_TARGET})

# On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
if (MSVC)
    target_compile_options (TestStandardLayoutMappingVisualizationOriginal PRIVATE /EHa)
endif ()
