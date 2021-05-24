# Declares global build options for Emergence project and applies their values.

include_guard (GLOBAL)

option (EMERGENCE_TREAT_WARNINGS_AS_ERRORS "Enables \"treat warnings as errors\" compiler policy for all targets." ON)

if (EMERGENCE_TREAT_WARNINGS_AS_ERRORS)
    if (MSVC)
        add_compile_options (/W4 /WX)
    else ()
        add_compile_options (-Wall -Wextra -pedantic -Werror)
    endif ()
endif ()