# Searches for Urho3D engine SDK and creates Urho3D library target on success.

if (DEFINED ENV{URHO3D_DIR})
    file (TO_CMAKE_PATH "$ENV{URHO3D_DIR}" CONVERTED_PATH)
    list (APPEND CMAKE_MODULE_PATH "${CONVERTED_PATH}/share/CMake/Modules")
    set (URHO3D_HOME "${CONVERTED_PATH}")
endif ()

set (CMAKE_FIND_DEBUG_MODE ON)
set (URHO3D_LIB_TYPE SHARED)
find_package (Urho3D)

if (URHO3D_FOUND)
    add_library (Urho3D SHARED IMPORTED)
    set (URHO3D_SHARED "${URHO3D_HOME}/bin/${CMAKE_SHARED_LIBRARY_PREFIX}Urho3D${CMAKE_SHARED_LIBRARY_SUFFIX}")

    if (NOT EXISTS "${URHO3D_SHARED}")
        set (URHO3D_SHARED "${URHO3D_HOME}/bin/${CMAKE_SHARED_LIBRARY_PREFIX}Urho3D_d${CMAKE_SHARED_LIBRARY_SUFFIX}")
        if (NOT EXISTS "${URHO3D_SHARED}")
            message (FATAL_ERROR "Unable to find Urho3D shared library!")
        endif ()
    endif ()

    set_target_properties (Urho3D PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${URHO3D_INCLUDE_DIRS}"
            IMPORTED_LOCATION "${URHO3D_SHARED}"
            IMPORTED_IMPLIB "${URHO3D_LIBRARIES}")

    add_custom_target (RunUrho3DEditor "${URHO3D_HOME}/bin/Editor" WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endif ()
