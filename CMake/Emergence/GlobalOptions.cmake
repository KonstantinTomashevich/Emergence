# Declares global build options for Emergence project and applies their values.

include_guard (GLOBAL)

option (EMERGENCE_COMPILE_TIME_TRACE "Requests compiler to print time trace. Supported only on CLang." OFF)
option (EMERGENCE_ENABLE_COVERAGE "Add compile and link time flags, that enable code coverage reporting." OFF)
option (EMERGENCE_INCLUDE_GPU_DEPENDANT_TESTS
        "Specifies whether GPU dependant tests should be registered and built." ON)
option (EMERGENCE_TREAT_WARNINGS_AS_ERRORS "Enables \"treat warnings as errors\" compiler policy for all targets." ON)

# We can not add common compile options here, because they would affect third party libraries compilation.
# Therefore every Emergence root source directory must call this function to setup compile options locally.
function (add_common_compile_options)
    if (EMERGENCE_COMPILE_TIME_TRACE)
        if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "^.*Clang$")
            add_compile_options (-ftime-trace)
        else ()
            # TODO: Alternatives for other compilers?
            message (FATAL_ERROR "Currently time tracing is supported only for CLang.")
        endif ()
    endif ()

    if (EMERGENCE_ENABLE_COVERAGE)
        if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "^.*Clang$")
            add_compile_options (-fprofile-instr-generate -fcoverage-mapping)
        else ()
            add_compile_options (--coverage)
            add_link_options (--coverage)
        endif ()
    endif ()

    if (EMERGENCE_TREAT_WARNINGS_AS_ERRORS)
        if (MSVC)
            add_compile_options (
                    /W4 /WX
                    # Zero length arrays greatly increase readability for classes and structs with dynamic sizes.
                    /wd4200
                    # Anonymous structs increase readability in some cases.
                    /wd4201
                    # Assignments inside conditional statements increase readability in some cases.
                    /wd4706)
        else ()
            add_compile_options (-Wall -Wextra -Werror)

            if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "^.*Clang$")
                # Exceptions in CLang format. Also, pedantic is enabled only on CLang,
                # because there is no pedantic exceptions on GCC.
                add_compile_options (
                        -pedantic
                        # Anonymous structs increase readability in some cases.
                        -Wno-gnu-anonymous-struct
                        # Nested anonymous types are allowed, because they are useful with unions.
                        -Wno-nested-anon-types
                        # Zero length arrays greatly increase readability for classes and structs with dynamic sizes.
                        -Wno-zero-length-array
                        # In some cases zero-arguments variadics are intentional and allows better customizations.
                        -Wno-gnu-zero-variadic-macro-arguments
                        # In most cases, using offsetof on non-POD types is safe nowadays, therefore we disable this warning.
                        -Wno-invalid-offsetof
                        # Sometimes CLang 14 incorrectly parses directory names for some reason and false positives this warning.
                        -Wno-nonportable-include-path)
            else ()
                # Exceptions in GCC format.
                add_compile_options (
                        # Used by XXHash and in some other places, that rely on block_cast.
                        -Wno-array-bounds
                        # In most cases, using offsetof on non-POD types is safe nowadays, therefore we disable this warning.
                        -Wno-invalid-offsetof)
            endif ()
        endif ()
    endif ()

    if (NOT MSVC)
        # On GCC we need to link STL and math everywhere.
        # It seems easier to do it globally here instead of adding this code to every target.
        link_libraries (m stdc++)
    endif ()
endfunction ()

# Debug iterators and containers are not only slow, but they also eat lots of memory and force
# service APIs to request additional memory for service iterators. Therefore they are disabled.
# We add this flag even to ThirdParty compilation to avoid link-time mismatches.
add_compile_definitions (_ITERATOR_DEBUG_LEVEL=0)
