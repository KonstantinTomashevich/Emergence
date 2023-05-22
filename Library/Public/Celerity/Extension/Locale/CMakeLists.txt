sober_library_begin (CelerityLocale STATIC)
    file (GLOB_RECURSE HEADERS "*.hpp")
    file (GLOB_RECURSE SOURCES "*.cpp")
    list (APPEND SOURCES ${HEADERS})

    sober_library_set_sources ("${SOURCES}")
    sober_library_include_directory (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

    sober_library_use_service (EXPECTED PRIVATE JobDispatcher)
    sober_library_use_service (EXPECTED PRIVATE Log)
    sober_library_use_service (EXPECTED PUBLIC ResourceProvider)

    sober_naming_variant_target (Celerity Original CELERITY_LIBRARY_TARGET)

    sober_library_link_library (PRIVATE "${SERIALIZATION_LIBRARY_TARGET}")
    sober_library_link_library (PUBLIC "${CELERITY_LIBRARY_TARGET}")

    sober_variant_begin (Original)
    sober_variant_end ()
sober_library_end ()
