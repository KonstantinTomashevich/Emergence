sober_library_begin (FileSystemTestUtility OBJECT)
    file (GLOB_RECURSE HEADERS *.hpp)
    file (GLOB_RECURSE SOURCES *.cpp)
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

    sober_naming_variant_target (Container Original CONTAINER_LIBRARY_TARGET)
    sober_library_link_library (PUBLIC "${CONTAINER_LIBRARY_TARGET}")

    sober_library_use_service (PER_VARIANT PRIVATE Testing)

    # On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
    if (MSVC)
        sober_library_private_compile_options (/EHs)
    endif ()

    sober_variant_begin (Doctest)
        sober_variant_freeze_implementation (Testing Doctest)
    sober_variant_end ()
sober_library_end ()
