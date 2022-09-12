file (GLOB_RECURSE SOURCES *.cpp)
file (GLOB_RECURSE HEADERS *.hpp)

add_executable (TestCelerityAssetConfigOriginal ${SOURCES} ${HEADERS})
add_test (NAME "TestCelerityAssetConfigOriginal" COMMAND TestCelerityAssetConfigOriginal)
add_dependencies (LibraryTests TestCelerityAssetConfigOriginal)
target_include_directories (TestCelerityAssetConfigOriginal PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

sober_naming_variant_target (CelerityAssetConfig Original CELERITY_ASSET_CONFIG_LIBRARY_TARGET)

target_link_libraries (TestCelerityAssetConfigOriginal PRIVATE ${CELERITY_ASSET_CONFIG_LIBRARY_TARGET})
link_celerity_with_dependencies (TestCelerityAssetConfigOriginal)

# On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
if (MSVC)
    target_compile_options (TestCelerityAssetConfigOriginal PRIVATE /EHa)
endif ()
