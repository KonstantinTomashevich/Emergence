sober_test_begin (StandardLayoutMapping)
    file (GLOB_RECURSE SOURCES *.cpp)
    file (GLOB_RECURSE HEADERS *.hpp)
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
    sober_library_link_library (PRIVATE Boost::unit_test_framework)

    sober_test_variant_begin (Original)
    sober_test_variant_end ()
sober_test_end ()

# TODO: Enable exceptions for Boost.Test on CLang compilers.