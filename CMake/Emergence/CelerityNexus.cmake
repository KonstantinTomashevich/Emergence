# Adds targets for CelerityNexus integration to currently registered executable,
# including target for hot reload and bootstrap configuration file generation.
#
# Arguments:
# - LOGIC: Name of the shared library target with logical part.
# - MODEL: Name of the shared library target with model part.
function (executable_integrate_celerity_nexus)
    if (NOT DEFINED ARTEFACT_NAME)
        message (FATAL_ERROR "Expected to be called as part of executable configuration routine!")
    endif ()

    cmake_parse_arguments (NEXUS "" "LOGIC;MODEL" "" ${ARGV})
    if (DEFINED SETUP_UNPARSED_ARGUMENTS OR
            NOT DEFINED NEXUS_LOGIC OR
            NOT DEFINED NEXUS_MODEL)
        message (FATAL_ERROR "Incorrect function arguments!")
    endif ()

    add_custom_target ("${ARTEFACT_NAME}HotReload")
    add_dependencies ("${ARTEFACT_NAME}" "${ARTEFACT_NAME}HotReload")

    set (HOT_RELOAD_LIBRARIES)
    list (APPEND HOT_RELOAD_LIBRARIES ${NEXUS_LOGIC} ${NEXUS_MODEL})

    find_linked_shared_libraries (TARGET "${NEXUS_LOGIC}" OUTPUT LOGIC_REQUIRED_LIBRARIES)
    list (APPEND HOT_RELOAD_LIBRARIES ${LOGIC_REQUIRED_LIBRARIES})

    find_linked_shared_libraries (TARGET "${NEXUS_MODEL}" OUTPUT MODEL_REQUIRED_LIBRARIES)
    list (APPEND HOT_RELOAD_LIBRARIES ${MODEL_REQUIRED_LIBRARIES})

    list (REMOVE_DUPLICATES HOT_RELOAD_LIBRARIES)
    foreach (LIBRARY_TARGET ${HOT_RELOAD_LIBRARIES})
        setup_shared_library_copy (
                LIBRARY "${LIBRARY_TARGET}"
                USER "${ARTEFACT_NAME}HotReload"
                OUTPUT "$<TARGET_FILE_DIR:${ARTEFACT_NAME}>")
    endforeach ()

    set (BOOTSTRAP_CONFIG)
    string (APPEND BOOTSTRAP_CONFIG "# NexusBootstrap\n")

    if (UNIX)
        string (APPEND BOOTSTRAP_CONFIG "modelPath: $<TARGET_SONAME_FILE_NAME:${NEXUS_MODEL}>\n")
        string (APPEND BOOTSTRAP_CONFIG "logicPath: $<TARGET_SONAME_FILE_NAME:${NEXUS_LOGIC}>\n")
    else ()
        string (APPEND BOOTSTRAP_CONFIG "modelPath: $<TARGET_FILE_NAME:${NEXUS_MODEL}>\n")
        string (APPEND BOOTSTRAP_CONFIG "logicPath: $<TARGET_FILE_NAME:${NEXUS_LOGIC}>\n")
    endif ()

    set (DEVELOPMENT_BOOTSTRAP_CONFIG "${BOOTSTRAP_CONFIG}hotReloadEnabled: 1\n")
    set (PACKAGED_BOOTSTRAP_CONFIG "${BOOTSTRAP_CONFIG}hotReloadEnabled: 0\n")

    file (GENERATE
            OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/CelerityNexusBootstrap.${ARTEFACT_NAME}.Development.yaml"
            CONTENT "${DEVELOPMENT_BOOTSTRAP_CONFIG}")

    file (GENERATE
            OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/CelerityNexusBootstrap.${ARTEFACT_NAME}.Packaged.yaml"
            CONTENT "${PACKAGED_BOOTSTRAP_CONFIG}")

    add_custom_target ("${ARTEFACT_NAME}AddNexusBootstrap"
            COMMENT "Add CelerityNexusBootstrap.yaml to ${ARTEFACT_NAME}."
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/CelerityNexusBootstrap.${ARTEFACT_NAME}.Development.yaml"
            "$<TARGET_FILE_DIR:${ARTEFACT_NAME}>/CelerityNexusBootstrap.yaml"
            VERBATIM)
    add_dependencies ("${ARTEFACT_NAME}" "${ARTEFACT_NAME}AddNexusBootstrap")

    if (UNIX)
        set_target_properties ("${NEXUS_LOGIC}" PROPERTIES BUILD_RPATH "\$ORIGIN;\$ORIGIN/..")
        set_target_properties ("${NEXUS_MODEL}" PROPERTIES BUILD_RPATH "\$ORIGIN;\$ORIGIN/..")
    endif ()
endfunction ()

# Adds CelerityNexus bootstrap configuration file copying to packaging group.
# Arguments:
# - NAME: Name of the packaging group.
# - EXECUTABLE: Name of the executable that was integrated with Nexus and is part of packaging group.
# - PACKAGING_OUTPUT: Output directory of the packaging group.
function (celerity_nexus_add_bootstrap_to_package)
    cmake_parse_arguments (NEXUS "" "NAME;EXECUTABLE;PACKAGING_OUTPUT" "" ${ARGV})
    if (DEFINED SETUP_UNPARSED_ARGUMENTS OR
            NOT DEFINED NEXUS_NAME OR
            NOT DEFINED NEXUS_EXECUTABLE OR
            NOT DEFINED NEXUS_PACKAGING_OUTPUT)
        message (FATAL_ERROR "Incorrect function arguments!")
    endif ()

    add_custom_target ("${NEXUS_NAME}AddNexusBootstrapToPackaging"
            COMMENT "Add CelerityNexusBootstrap.yaml to ${NEXUS_NAME} package."
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/CelerityNexusBootstrap.${NEXUS_EXECUTABLE}.Packaged.yaml"
            "${NEXUS_PACKAGING_OUTPUT}/CelerityNexusBootstrap.yaml"
            VERBATIM)
    add_dependencies ("${NEXUS_NAME}AddNexusBootstrapToPackaging" "${NEXUS_NAME}PackageClean" )
    add_dependencies ("${NEXUS_NAME}Packaging" "${NEXUS_NAME}AddNexusBootstrapToPackaging")
endfunction ()
