file (GLOB_RECURSE SOURCES *.cpp)
file (GLOB_RECURSE HEADERS *.hpp)

add_executable (TestCelerityResourceConfigOriginal ${SOURCES} ${HEADERS})
add_test (NAME "TestCelerityResourceConfigOriginal" COMMAND TestCelerityResourceConfigOriginal)
add_dependencies (LibraryTests TestCelerityResourceConfigOriginal)
target_include_directories (TestCelerityResourceConfigOriginal PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

sober_naming_implementation_target (ResourceProvider Original RESOURCE_PROVIDER_LIBRARY_TARGET)
sober_naming_implementation_target (VirtualFileSystem Original VIRTUAL_FILE_SYSTEM_IMPLEMENTATION_TARGET)
sober_naming_variant_target (CelerityResourceConfig Original CELERITY_RESOURCE_CONFIG_LIBRARY_TARGET)
sober_naming_variant_target (Serialization Original SERIALIZATION_LIBRARY_TARGET)

sober_target_link_libraries (TestCelerityResourceConfigOriginal PRIVATE
        ${RESOURCE_PROVIDER_LIBRARY_TARGET}
        ${VIRTUAL_FILE_SYSTEM_IMPLEMENTATION_TARGET}
        ${CELERITY_RESOURCE_CONFIG_LIBRARY_TARGET}
        ${SERIALIZATION_LIBRARY_TARGET})
link_celerity_with_dependencies (TestCelerityResourceConfigOriginal)

# On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
if (MSVC)
    target_compile_options (TestCelerityResourceConfigOriginal PRIVATE /EHa)
endif ()
