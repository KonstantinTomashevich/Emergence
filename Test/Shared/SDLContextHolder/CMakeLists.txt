sober_library_begin (SDLContextHolder OBJECT)
    file (GLOB_RECURSE HEADERS *.hpp)
    file (GLOB_RECURSE SOURCES *.cpp)
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

    sober_library_link_library (PUBLIC SDL3::SDL3)

    sober_library_use_service (PER_VARIANT PRIVATE RenderBackend)

    # On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
    if (MSVC)
        sober_library_private_compile_options (/EHs)
    endif ()

    sober_variant_begin (Original)
    sober_variant_end ()
sober_library_end ()
