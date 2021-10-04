file (GLOB_RECURSE SOURCES *.cpp)
file (GLOB_RECURSE HEADERS *.hpp)

add_executable (TestFlowOriginal ${SOURCES} ${HEADERS})
add_test (NAME "TestFlowOriginal" COMMAND TestFlowOriginal)
add_dependencies (LibraryTests TestFlowOriginal)
target_include_directories (TestFlowOriginal PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

sober_naming_variant_target (Flow Original FLOW_LIBRARY_TARGET)
sober_naming_variant_target (Testing Doctest TESTING_LIBRARY_TARGET)
target_link_libraries (TestFlowOriginal PRIVATE ${FLOW_LIBRARY_TARGET} ${TESTING_LIBRARY_TARGET})

# On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
if (MSVC)
    target_compile_options (TestFlowOriginal PRIVATE /EHa)
endif ()
