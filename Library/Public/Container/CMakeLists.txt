sober_library_begin (Container OBJECT)
    # Services use Container and Container uses services, therefore we end up with
    # ContainerBase dependency on ContainerOriginal, that breaks the build.
    # Therefore, we git rid of ContainerBase by enforcing compile time variants.
    sober_library_forbid_link_variants ()

    file (GLOB_RECURSE HEADERS "*.hpp")
    file (GLOB_RECURSE SOURCES "*.cpp")
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

    sober_library_link_library (PUBLIC Boost::unordered)

    sober_library_use_service (EXPECTED PUBLIC Assert)
    sober_library_use_service (EXPECTED PUBLIC Memory)
    sober_library_use_service (EXPECTED PUBLIC StandardLayoutMapping)

    sober_variant_begin (Original)
    sober_variant_end ()
sober_library_end ()
