# Searches for NVIDIA PhysX SDK and creates PhysX library target on success.

if (DEFINED ENV{PHYSX_DIR})
    file (TO_CMAKE_PATH "$ENV{PHYSX_DIR}" PHYSX_HOME)
endif ()

if (DEFINED PHYSX_HOME)
    message (STATUS "Searching for PhysX in ${PHYSX_HOME}...")

    function (check_physx_requirement REQUIREMENT)
        if (NOT EXISTS "${REQUIREMENT}")
            message (STATUS "    ${REQUIREMENT} is missing.")
            set (PHYSX_FOUND 0 PARENT_SCOPE)
        endif ()
    endfunction ()

    set (PHYSX_ARCH "64" CACHE STRING "Architecture-specific suffix of PhysX libraries.")
    set (PHYSX_MAIN_RUNTIME PhysX)
    set (PHYSX_MAIN_STATIC PhysX)
    set (PHYSX_COMMON_RUNTIME PhysXCommon)
    set (PHYSX_COMMON_STATIC PhysXCommon)
    set (PHYSX_FOUNDATION_RUNTIME PhysXFoundation)
    set (PHYSX_FOUNDATION_STATIC PhysXFoundation)
    set (PHYSX_EXTENSIONS_STATIC PhysXExtensions_static)
    set (PHYSX_PVD_STATIC PhysXPvdSDK_static)

    set (PHYSX_FOUND 1)
    set (PHYSX_SHARED_LIBRARY_PREFIX "${PHYSX_HOME}/bin/${CMAKE_SHARED_LIBRARY_PREFIX}")
    set (PHYSX_STATIC_LIBRARY_PREFIX "${PHYSX_HOME}/bin/${CMAKE_STATIC_LIBRARY_PREFIX}")

    foreach (REQUIREMENT IN ITEMS "PHYSX_MAIN_RUNTIME" "PHYSX_COMMON_RUNTIME" "PHYSX_FOUNDATION_RUNTIME")
        set ("${REQUIREMENT}"
                "${PHYSX_SHARED_LIBRARY_PREFIX}${${REQUIREMENT}}_${PHYSX_ARCH}${CMAKE_SHARED_LIBRARY_SUFFIX}")
        check_physx_requirement ("${${REQUIREMENT}}")
    endforeach ()

    foreach (REQUIREMENT IN ITEMS "PHYSX_MAIN_STATIC" "PHYSX_COMMON_STATIC" "PHYSX_FOUNDATION_STATIC"
            "PHYSX_EXTENSIONS_STATIC" "PHYSX_PVD_STATIC")
        set ("${REQUIREMENT}"
                "${PHYSX_STATIC_LIBRARY_PREFIX}${${REQUIREMENT}}_${PHYSX_ARCH}${CMAKE_STATIC_LIBRARY_SUFFIX}")
        check_physx_requirement ("${${REQUIREMENT}}")
    endforeach ()

    set (PHYSX_INCLUDE "${PHYSX_HOME}/include")
    check_physx_requirement ("${PHYSX_INCLUDE}")

    if (PHYSX_FOUND)
        message (STATUS "Found NVIDIA PhysX:")
        message (STATUS "    Main runtime: ${PHYSX_MAIN_RUNTIME}.")
        message (STATUS "    Main static: ${PHYSX_MAIN_STATIC}.")
        message (STATUS "    Common runtime: ${PHYSX_COMMON_RUNTIME}.")
        message (STATUS "    Common static: ${PHYSX_COMMON_STATIC}.")
        message (STATUS "    Foundation runtime: ${PHYSX_FOUNDATION_RUNTIME}.")
        message (STATUS "    Foundation static: ${PHYSX_FOUNDATION_STATIC}.")
        message (STATUS "    Extensions static: ${PHYSX_EXTENSIONS_STATIC}.")
        message (STATUS "    PVD static: ${PHYSX_PVD_STATIC}.")
        message (STATUS "    Include: ${PHYSX_INCLUDE}.")

        add_library (PhysX SHARED IMPORTED)
        set_target_properties (PhysX PROPERTIES
                IMPORTED_LOCATION "${PHYSX_MAIN_RUNTIME}"
                IMPORTED_IMPLIB "${PHYSX_MAIN_STATIC}"
                INTERFACE_INCLUDE_DIRECTORIES "${PHYSX_INCLUDE}")

        add_library (PhysX::Common SHARED IMPORTED)
        set_target_properties (PhysX::Common PROPERTIES
                IMPORTED_LOCATION "${PHYSX_COMMON_RUNTIME}"
                IMPORTED_IMPLIB "${PHYSX_COMMON_STATIC}")

        add_library (PhysX::Foundation SHARED IMPORTED)
        set_target_properties (PhysX::Foundation PROPERTIES
                IMPORTED_LOCATION "${PHYSX_FOUNDATION_RUNTIME}"
                IMPORTED_IMPLIB "${PHYSX_FOUNDATION_STATIC}")

        add_library (PhysX::PVD STATIC IMPORTED)
        set_target_properties (PhysX::PVD PROPERTIES IMPORTED_LOCATION "${PHYSX_PVD_STATIC}")

        add_library (PhysX::Extensions STATIC IMPORTED)
        set_target_properties (PhysX::Extensions PROPERTIES IMPORTED_LOCATION "${PHYSX_EXTENSIONS_STATIC}")

        reflected_target_link_libraries (
                TARGET PhysX INTERFACE PhysX::Common PhysX::Foundation PhysX::PVD PhysX::Extensions)
    endif ()
endif ()
