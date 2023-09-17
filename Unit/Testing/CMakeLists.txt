set (TESTING_BACKEND "Doctest" CACHE STRING "Name of backend testing framework to use across Emergence.")
register_interface (Testing)
interface_include ("${CMAKE_CURRENT_SOURCE_DIR}/Common")

if (TESTING_BACKEND STREQUAL "Doctest")
    interface_include ("${CMAKE_CURRENT_SOURCE_DIR}/Doctest")
    interface_require (THIRD_PARTY doctest)

    # On MSVC CLang exceptions are disabled by default, but Doctest needs them for REQUIRE assertions.
    if (MSVC)
        interface_compile_options (/EHa)
    endif ()
else ()
    message (SEND_ERROR "Unknown testing backend \"${TESTING_BACKEND}\".")
endif ()
