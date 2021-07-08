# Declares global build options for Emergence project and applies their values.

include_guard (GLOBAL)

option (EMERGENCE_TREAT_WARNINGS_AS_ERRORS "Enables \"treat warnings as errors\" compiler policy for all targets." ON)
option (EMERGENCE_COMPILE_TIME_TRACE "Requests compiler to print time trace. Supported only on CLang." OFF)

if (EMERGENCE_TREAT_WARNINGS_AS_ERRORS)
    if (MSVC)
        add_compile_options (/W4 /WX)
    else ()
        add_compile_options (-Wall -Wextra -pedantic -Werror)
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