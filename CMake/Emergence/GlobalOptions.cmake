# Declares global build options for Emergence project and applies their values.

include_guard (GLOBAL)

option (EMERGENCE_TREAT_WARNINGS_AS_ERRORS "Enables \"treat warnings as errors\" compiler policy for all targets." ON)
option (EMERGENCE_COMPILE_TIME_TRACE "Requests compiler to print time trace. Supported only on CLang." OFF)
option (EMERGENCE_ENABLE_COVERAGE "Add compile and link time flags, that enable code coverage reporting." OFF)

if (EMERGENCE_TREAT_WARNINGS_AS_ERRORS)
    if (MSVC)
        add_compile_options (
                /W4 /WX
                # Zero length arrays greatly increase readability classes and structs with dynamic sizes.
                /wd4200
                # Anonymous structs increase readability in some cases.
                /wd4201)
    else ()
        add_compile_options (
                -Wall -Wextra -Werror
                # Allow raw memory access for classes. It's need for some low level optimizations in reflection.
                -Wno-error=class-memaccess)

        # Exceptions can be added to pedantic mode only on CLang.
        if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "^.*Clang$")
            add_compile_options (
                    -pedantic
                    # Anonymous structs increase readability in some cases.
                    -Wno-gnu-anonymous-struct
                    # Nested anonymous types are allowed, because they are useful with unions.
                    -Wno-nested-anon-types
                    # Zero length arrays greatly increase readability classes and structs with dynamic sizes.
                    -Wno-zero-length-array)
        endif ()
    endif ()
endif ()

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