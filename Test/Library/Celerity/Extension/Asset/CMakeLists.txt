file (GLOB_RECURSE SOURCES *.cpp)
file (GLOB_RECURSE HEADERS *.hpp)

add_executable (TestCelerityAssetOriginal ${SOURCES} ${HEADERS})
add_test (NAME "TestCelerityAssetOriginal" COMMAND TestCelerityAssetOriginal)
add_dependencies (LibraryTests TestCelerityAssetOriginal)
target_include_directories (TestCelerityAssetOriginal PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

sober_naming_variant_target (CelerityAsset Original CELERITY_ASSET_LIBRARY_TARGET)
target_link_libraries (TestCelerityAssetOriginal PRIVATE ${CELERITY_ASSET_LIBRARY_TARGET})
link_celerity_with_dependencies (TestCelerityAssetOriginal)

# On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
if (MSVC)
    target_compile_options (TestCelerityAssetOriginal PRIVATE /EHa)
endif ()
