file (GLOB_RECURSE SOURCES *.cpp)
file (GLOB_RECURSE HEADERS *.hpp)

add_executable (TestCelerityOriginal ${SOURCES} ${HEADERS})
add_test (NAME "TestCelerityOriginal" COMMAND TestCelerityOriginal)
add_dependencies (LibraryTests TestCelerityOriginal)
target_include_directories (TestCelerityOriginal PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
link_celerity_with_dependencies (TestCelerityOriginal)

# On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
if (MSVC)
    target_compile_options (TestCelerityOriginal PRIVATE /EHa)
endif ()
