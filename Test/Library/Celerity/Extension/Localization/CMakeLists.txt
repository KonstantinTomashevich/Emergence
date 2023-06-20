file (GLOB_RECURSE SOURCES *.cpp)
file (GLOB_RECURSE HEADERS *.hpp)

add_executable (TestCelerityLocaleOriginal ${SOURCES} ${HEADERS})
add_test (NAME "TestCelerityLocaleOriginal" COMMAND TestCelerityLocaleOriginal)
add_dependencies (LibraryTests TestCelerityLocaleOriginal)
target_include_directories (TestCelerityLocaleOriginal PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

sober_naming_implementation_target (ResourceProvider Original RESOURCE_PROVIDER_IMPLEMENTATION_TARGET)
sober_naming_implementation_target (VirtualFileSystem Original VIRTUAL_FILE_SYSTEM_IMPLEMENTATION_TARGET)
sober_naming_variant_target (CelerityLocale Original CELERITY_LOCALE_LIBRARY_TARGET)
sober_naming_variant_target (Serialization Original SERIALIZATION_LIBRARY_TARGET)

sober_target_link_libraries (TestCelerityLocaleOriginal PRIVATE
        ${RESOURCE_PROVIDER_IMPLEMENTATION_TARGET} ${VIRTUAL_FILE_SYSTEM_IMPLEMENTATION_TARGET}
        ${CELERITY_LOCALE_LIBRARY_TARGET} ${SERIALIZATION_LIBRARY_TARGET})
link_celerity_with_dependencies (TestCelerityLocaleOriginal)

# On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
if (MSVC)
    target_compile_options (TestCelerityLocaleOriginal PRIVATE /EHa)
endif ()
