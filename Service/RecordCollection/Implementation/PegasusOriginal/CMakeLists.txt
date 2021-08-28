sober_implementation_begin (PegasusOriginal)
    file (GLOB_RECURSE SOURCES *.cpp)
    file (GLOB_RECURSE HEADERS *.hpp)
    list (APPEND SOURCES ${HEADERS})

    sober_implementation_setup_target (STATIC "${SOURCES}")
    sober_implementation_include_directory ("${CMAKE_CURRENT_SOURCE_DIR}")

    sober_naming_variant_target (SyntaxSugar Original SYNTAX_SUGAR_LIBRARY_TARGET)
    sober_implementation_link_library ("${SYNTAX_SUGAR_LIBRARY_TARGET}")

    sober_naming_variant_target (Pegasus Original PEGASUS_LIBRARY_TARGET)
    sober_implementation_link_library ("${PEGASUS_LIBRARY_TARGET}")
sober_implementation_end ()
