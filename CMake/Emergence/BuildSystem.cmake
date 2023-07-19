define_property (TARGET PROPERTY INTERFACE_LINKED_TARGETS
        BRIEF_DOCS "Targets linked in INTERFACE scope to this target using reflected_target_link_libraries."
        FULL_DOCS "We use reflected_target_link_libraries in order to make it easy to traverse linking hierarchy.")

define_property (TARGET PROPERTY PUBLIC_LINKED_TARGETS
        BRIEF_DOCS "Targets linked in PUBLIC scope to this target using reflected_target_link_libraries."
        FULL_DOCS "We use reflected_target_link_libraries in order to make it easy to traverse linking hierarchy.")

define_property (TARGET PROPERTY PRIVATE_LINKED_TARGETS
        BRIEF_DOCS "Targets linked in PRIVATE scope to this target using reflected_target_link_libraries."
        FULL_DOCS "We use reflected_target_link_libraries in order to make it easy to traverse linking hierarchy.")

# Adapter for target_link_libraries that keeps linked libraries accessible in appropriate target properties.
# Arguments:
# - TARGET: target to which we are linking.
# - INTERFACE: targets that are linked to it in INTERFACE scope.
# - PUBLIC: targets that are linked to it in PUBLIC scope.
# - PRIVATE: targets that are linked to it in PRIVATE scope.
function (reflected_target_link_libraries)
    cmake_parse_arguments (LINK "" "TARGET" "INTERFACE;PUBLIC;PRIVATE" ${ARGV})
    if (DEFINED LINK_UNPARSED_ARGUMENTS OR
            NOT DEFINED LINK_TARGET OR (
            NOT DEFINED LINK_INTERFACE AND
            NOT DEFINED LINK_PUBLIC AND
            NOT DEFINED LINK_PRIVATE))
        message (FATAL_ERROR "Incorrect function arguments!")
    endif ()

    foreach (SCOPE INTERFACE PUBLIC PRIVATE)
        if (DEFINED LINK_${SCOPE})

            target_link_libraries ("${LINK_TARGET}" "${SCOPE}" ${LINK_${SCOPE}})
            get_target_property (LINKED_TARGETS "${LINK_TARGET}" "${SCOPE}_LINKED_TARGETS")

            if (LINKED_TARGETS STREQUAL "LINKED_TARGETS-NOTFOUND")
                set (LINKED_TARGETS)
            endif ()

            list (APPEND LINKED_TARGETS ${LINK_${SCOPE}})
            set_target_properties ("${LINK_TARGET}" PROPERTIES "${SCOPE}_LINKED_TARGETS" "${LINKED_TARGETS}")

        endif ()
    endforeach ()

endfunction ()

# Recursively searches for linked targets of given target and outputs them to given variable.
# Arguments:
# - TARGET: root target to start the search.
# - OUTPUT: name of the output variable to store the found targets.
# - ARTEFACT_SCOPE: option, if passed, search will not exit artefact (shared library or executable) scope.
# - CHECK_VISIBILITY: option, if passed, only targets that are directly visible to root target will be added to list.
function (find_linked_targets_recursively)
    cmake_parse_arguments (SEARCH "ARTEFACT_SCOPE;CHECK_VISIBILITY" "TARGET;OUTPUT" "" ${ARGV})
    if (DEFINED SEARCH_UNPARSED_ARGUMENTS OR
            NOT DEFINED SEARCH_TARGET OR
            NOT DEFINED SEARCH_OUTPUT)
        message (FATAL_ERROR "Incorrect function arguments!")
    endif ()

    set (ALL_LINKED_TARGETS)
    set (SCAN_QUEUE)
    list (APPEND SCAN_QUEUE ${SEARCH_TARGET})
    list (LENGTH SCAN_QUEUE SCAN_QUEUE_LENGTH)

    while (SCAN_QUEUE_LENGTH GREATER 0)
        list (POP_BACK SCAN_QUEUE ITEM)
        if (TARGET ${ITEM})
            set (CHECK_SCOPES)
            list (APPEND CHECK_SCOPES INTERFACE PUBLIC)

            if (NOT SEARCH_CHECK_VISIBILITY OR ITEM STREQUAL SEARCH_TARGET)
                list (APPEND CHECK_SCOPES PRIVATE)
            endif ()

            foreach (SCOPE ${CHECK_SCOPES})
                get_target_property (LINKED_TARGETS ${ITEM} "${SCOPE}_LINKED_TARGETS")
                if (NOT "${LINKED_TARGETS}" STREQUAL "LINKED_TARGETS-NOTFOUND")
                    foreach (LINKED_TARGET ${LINKED_TARGETS})
                        if (TARGET ${LINKED_TARGET})

                            get_target_property (TARGET_TYPE "${LINKED_TARGET}" TYPE)
                            if (NOT SEARCH_ARTEFACT_SCOPE OR NOT TARGET_TYPE STREQUAL "SHARED_LIBRARY")
                                list (FIND ALL_LINKED_TARGETS "${LINKED_TARGET}" LINKED_TARGET_INDEX)
                                if (LINKED_TARGET_INDEX EQUAL -1)
                                    list (APPEND ALL_LINKED_TARGETS ${LINKED_TARGET})
                                    list (APPEND SCAN_QUEUE ${LINKED_TARGET})
                                endif ()
                            endif ()

                        else ()
                            message (WARNING "Unable to find linked target \"${LINKED_TARGET}\".")
                        endif ()
                    endforeach ()
                endif ()
            endforeach ()

        else ()
            message (WARNING "Unable to find linked target \"${ITEM}\".")
        endif ()

        list (LENGTH SCAN_QUEUE SCAN_QUEUE_LENGTH)
    endwhile ()

    set ("${SEARCH_OUTPUT}" "${ALL_LINKED_TARGETS}" PARENT_SCOPE)
endfunction ()

# Recursively searches for linked shared libraries that are needed for given target.
# Arguments:
# - TARGET: target for which we're searching.
# - OUTPUT: name of the output variable to store the found shared libraries.
function (find_linked_shared_libraries)
    cmake_parse_arguments (SEARCH "" "TARGET;OUTPUT" "" ${ARGV})
    if (DEFINED SEARCH_UNPARSED_ARGUMENTS OR
            NOT DEFINED SEARCH_TARGET OR
            NOT DEFINED SEARCH_OUTPUT)
        message (FATAL_ERROR "Incorrect function arguments!")
    endif ()

    set (LIBRARIES)
    find_linked_targets_recursively (TARGET "${SEARCH_TARGET}" OUTPUT ALL_LINKED_TARGETS)

    foreach (LINKED_TARGET ${ALL_LINKED_TARGETS})
        get_target_property (TARGET_TYPE "${LINKED_TARGET}" TYPE)
        if (TARGET_TYPE STREQUAL "SHARED_LIBRARY")
            list (APPEND LIBRARIES "${LINKED_TARGET}")
        endif ()
    endforeach ()

    set ("${SEARCH_OUTPUT}" "${LIBRARIES}" PARENT_SCOPE)
endfunction ()

define_property (TARGET PROPERTY UNIT_TARGET_TYPE
        BRIEF_DOCS "Type of the registered unit target."
        FULL_DOCS "Supported values: Abstract, Concrete, ConcreteInterface, Interface.")

# Starts configuration routine of interface unit: header-only library.
function (register_interface UNIT_NAME)
    message (STATUS "Registering interface \"${UNIT_NAME}\"...")
    add_library ("${UNIT_NAME}" INTERFACE)
    set_target_properties ("${UNIT_NAME}" PROPERTIES UNIT_TARGET_TYPE "Interface")
    set (UNIT_NAME "${UNIT_NAME}" PARENT_SCOPE)
endfunction ()

# Adds given directories to interface include list of current interface unit.
function (interface_include)
    foreach (INCLUDE_DIR ${ARGV})
        message (STATUS "    Add include \"${INCLUDE_DIR}\".")
        target_include_directories ("${UNIT_NAME}" INTERFACE ${INCLUDE_DIR})
    endforeach ()
endfunction ()

# Registers requirements of current interface unit.
# Arguments:
# - ABSTRACT: list of required abstract units.
# - CONCRETE_INTERFACE: list of required interfaces of concrete units.
# - INTERFACE: list of required interface units.
# - THIRD_PARTY: list of required third party targets.
function (interface_require)
    cmake_parse_arguments (REQUIRE "" "" "ABSTRACT;CONCRETE_INTERFACE;INTERFACE;THIRD_PARTY" ${ARGV})
    if (DEFINED REQUIRE_UNPARSED_ARGUMENTS OR (
            NOT DEFINED REQUIRE_ABSTRACT AND
            NOT DEFINED REQUIRE_CONCRETE_INTERFACE AND
            NOT DEFINED REQUIRE_INTERFACE AND
            NOT DEFINED REQUIRE_THIRD_PARTY))
        message (FATAL_ERROR "Incorrect function arguments!")
    endif ()

    if (DEFINED REQUIRE_ABSTRACT)
        foreach (DEPENDENCY ${REQUIRE_ABSTRACT})
            message (STATUS "    Require abstract \"${DEPENDENCY}\".")
            reflected_target_link_libraries (TARGET "${UNIT_NAME}" INTERFACE "${DEPENDENCY}")
        endforeach ()
    endif ()

    if (DEFINED REQUIRE_CONCRETE_INTERFACE)
        foreach (DEPENDENCY ${REQUIRE_CONCRETE_INTERFACE})
            message (STATUS "    Require concrete interface \"${DEPENDENCY}\".")
            reflected_target_link_libraries (TARGET "${UNIT_NAME}" INTERFACE "${DEPENDENCY}Interface")
        endforeach ()
    endif ()

    if (DEFINED REQUIRE_INTERFACE)
        foreach (DEPENDENCY ${REQUIRE_INTERFACE})
            message (STATUS "    Require interface \"${DEPENDENCY}\".")
            reflected_target_link_libraries (TARGET "${UNIT_NAME}" INTERFACE "${DEPENDENCY}")
        endforeach ()
    endif ()

    if (DEFINED REQUIRE_THIRD_PARTY)
        foreach (DEPENDENCY ${REQUIRE_THIRD_PARTY})
            message (STATUS "    Require third party \"${DEPENDENCY}\".")
            reflected_target_link_libraries (TARGET "${UNIT_NAME}" INTERFACE "${DEPENDENCY}")
        endforeach ()
    endif ()
endfunction ()

# Adds interface compile options to current interface unit.
function (interface_compile_options)
    message (STATUS "    Add compile options \"${ARGV}\".")
    target_compile_options ("${UNIT_NAME}" INTERFACE ${ARGV})
endfunction ()

define_property (TARGET PROPERTY REQUIRED_CONCRETE_UNIT
        BRIEF_DOCS "Name of concrete unit for this concrete unit interface."
        FULL_DOCS "Concrete units consist of two targets: interface with only headers and implementation objects.")

# Starts configuration routine of concrete unit: headers with one concrete implementation.
function (register_concrete UNIT_NAME)
    message (STATUS "Registering concrete \"${UNIT_NAME}\"...")

    add_library ("${UNIT_NAME}Interface" INTERFACE)
    set_target_properties ("${UNIT_NAME}Interface" PROPERTIES
            UNIT_TARGET_TYPE "ConcreteInterface"
            REQUIRED_CONCRETE_UNIT "${UNIT_NAME}")

    add_library ("${UNIT_NAME}" OBJECT)
    set_target_properties ("${UNIT_NAME}" PROPERTIES UNIT_TARGET_TYPE "Concrete")
    reflected_target_link_libraries (TARGET "${UNIT_NAME}" PUBLIC "${UNIT_NAME}Interface")

    # Generate API header for shared library support.
    file (MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Generated")
    generate_api_header (
            API_MACRO "${UNIT_NAME}Api"
            EXPORT_MACRO "${UNIT_NAME}Implementation"
            OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/Generated/${UNIT_NAME}Api.hpp")
    target_include_directories ("${UNIT_NAME}Interface" INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/Generated")
    target_compile_definitions ("${UNIT_NAME}" PRIVATE "${UNIT_NAME}Implementation")

    set (UNIT_NAME "${UNIT_NAME}" PARENT_SCOPE)
endfunction ()

# Adds sources that match given glob recurse patterns to current concrete unit.
function (concrete_sources)
    foreach (PATTERN ${ARGV})
        message (STATUS "    Add sources with recurse pattern \"${PATTERN}\".")
        file (GLOB_RECURSE SOURCES "${PATTERN}")
        target_sources ("${UNIT_NAME}" PRIVATE ${SOURCES})
    endforeach ()
endfunction ()

# Adds include directories to current concrete unit.
# Arguments:
# - PUBLIC: directories that are meant to be publicly linked.
# - PRIVATE: directories that are meant to be privately linked.
function (concrete_include)
    cmake_parse_arguments (INCLUDE "" "" "PUBLIC;PRIVATE" ${ARGV})
    if (DEFINED INCLUDE_UNPARSED_ARGUMENTS OR (
            NOT DEFINED INCLUDE_PUBLIC AND
            NOT DEFINED INCLUDE_PRIVATE))
        message (FATAL_ERROR "Incorrect function arguments!")
    endif ()

    if (DEFINED INCLUDE_PUBLIC)
        foreach (INCLUDE_DIR ${INCLUDE_PUBLIC})
            message (STATUS "    Add public scope include \"${INCLUDE_DIR}\".")
            target_include_directories ("${UNIT_NAME}Interface" INTERFACE ${INCLUDE_DIR})
        endforeach ()
    endif ()

    if (DEFINED INCLUDE_PRIVATE)
        foreach (INCLUDE_DIR ${INCLUDE_PRIVATE})
            message (STATUS "    Add private scope include \"${INCLUDE_DIR}\".")
            target_include_directories ("${UNIT_NAME}" PRIVATE ${INCLUDE_DIR})
        endforeach ()
    endif ()
endfunction ()

# Registers requirements of current concrete unit.
# Arguments:
# - SCOPE: scope for these requirements, either PUBLIC or PRIVATE.
# - ABSTRACT: list of required abstract units.
# - CONCRETE_INTERFACE: list of required interfaces of concrete units.
# - INTERFACE: list of required interface units.
# - THIRD_PARTY: list of required third party targets.
function (concrete_require)
    cmake_parse_arguments (REQUIRE "" "SCOPE" "INTERFACE;CONCRETE_INTERFACE;ABSTRACT;THIRD_PARTY" ${ARGV})
    if (DEFINED REQUIRE_UNPARSED_ARGUMENTS OR
            NOT DEFINED REQUIRE_SCOPE OR (
            NOT DEFINED REQUIRE_INTERFACE AND
            NOT DEFINED REQUIRE_CONCRETE_INTERFACE AND
            NOT DEFINED REQUIRE_ABSTRACT AND
            NOT DEFINED REQUIRE_THIRD_PARTY))
        message (FATAL_ERROR "Incorrect function arguments!")
    endif ()

    if (REQUIRE_SCOPE STREQUAL "PUBLIC")

        if (DEFINED REQUIRE_INTERFACE)
            foreach (DEPENDENCY ${REQUIRE_INTERFACE})
                message (STATUS "    Require public scope interface \"${DEPENDENCY}\".")
                reflected_target_link_libraries (TARGET "${UNIT_NAME}Interface" INTERFACE "${DEPENDENCY}")
            endforeach ()
        endif ()

        if (DEFINED REQUIRE_CONCRETE_INTERFACE)
            foreach (DEPENDENCY ${REQUIRE_CONCRETE_INTERFACE})
                message (STATUS "    Require public scope interface of concrete \"${DEPENDENCY}\".")
                reflected_target_link_libraries (TARGET "${UNIT_NAME}Interface" INTERFACE "${DEPENDENCY}Interface")
            endforeach ()
        endif ()

        if (DEFINED REQUIRE_ABSTRACT)
            foreach (DEPENDENCY ${REQUIRE_ABSTRACT})
                message (STATUS "    Require public scope abstract \"${DEPENDENCY}\".")
                reflected_target_link_libraries (TARGET "${UNIT_NAME}Interface" INTERFACE "${DEPENDENCY}")
            endforeach ()
        endif ()

        if (DEFINED REQUIRE_THIRD_PARTY)
            foreach (DEPENDENCY ${REQUIRE_THIRD_PARTY})
                message (STATUS "    Require public scope third party \"${DEPENDENCY}\".")
                reflected_target_link_libraries (TARGET "${UNIT_NAME}Interface" INTERFACE "${DEPENDENCY}")
            endforeach ()
        endif ()

    elseif (REQUIRE_SCOPE STREQUAL "PRIVATE")

        if (DEFINED REQUIRE_INTERFACE)
            foreach (DEPENDENCY ${REQUIRE_INTERFACE})
                message (STATUS "    Require private scope interface \"${DEPENDENCY}\".")
                reflected_target_link_libraries (TARGET "${UNIT_NAME}" PRIVATE "${DEPENDENCY}")
            endforeach ()
        endif ()

        if (DEFINED REQUIRE_CONCRETE_INTERFACE)
            foreach (DEPENDENCY ${REQUIRE_CONCRETE_INTERFACE})
                message (STATUS "    Require private scope interface of concrete \"${DEPENDENCY}\".")
                reflected_target_link_libraries (TARGET "${UNIT_NAME}" PRIVATE "${DEPENDENCY}Interface")
            endforeach ()
        endif ()

        if (DEFINED REQUIRE_ABSTRACT)
            foreach (DEPENDENCY ${REQUIRE_ABSTRACT})
                message (STATUS "    Require private scope abstract \"${DEPENDENCY}\".")
                reflected_target_link_libraries (TARGET "${UNIT_NAME}" PRIVATE "${DEPENDENCY}")
            endforeach ()
        endif ()

        if (DEFINED REQUIRE_THIRD_PARTY)
            foreach (DEPENDENCY ${REQUIRE_THIRD_PARTY})
                message (STATUS "    Require private scope third party \"${DEPENDENCY}\".")
                reflected_target_link_libraries (TARGET "${UNIT_NAME}" PRIVATE "${DEPENDENCY}")
            endforeach ()
        endif ()

    else ()
        message (FATAL_ERROR "Incorrect function arguments!")
    endif ()
endfunction ()

# Adds compile options to current concrete unit.
# Arguments:
# - PUBLIC: compile options that are added to public scope.
# - PRIVATE: compile options that are added to private scope.
function (concrete_compile_options)
    cmake_parse_arguments (OPTIONS "" "" "PUBLIC;PRIVATE" ${ARGV})
    if (DEFINED OPTIONS_UNPARSED_ARGUMENTS OR (
            NOT DEFINED OPTIONS_PUBLIC AND
            NOT DEFINED OPTIONS_PRIVATE))
        message (FATAL_ERROR "Incorrect function arguments!")
    endif ()

    if (DEFINED OPTIONS_PUBLIC)
        message (STATUS "    Add public compile options \"${OPTIONS_PUBLIC}\".")
        target_compile_options ("${UNIT_NAME}Interface" INTERFACE ${OPTIONS_PUBLIC})
    endif ()

    if (DEFINED OPTIONS_PRIVATE)
        message (STATUS "    Add private compile options \"${OPTIONS_PRIVATE}\".")
        target_compile_options ("${UNIT_NAME}" PRIVATE ${OPTIONS_PRIVATE})
    endif ()
endfunction ()

# Informs build system that this concrete unit implements given abstract unit.
# Needed to pass correct compile definitions to concrete unit objects.
function (concrete_implements_abstract ABSTRACT_NAME)
    message (STATUS "    Implement abstract unit \"${ABSTRACT_NAME}\".")
    reflected_target_link_libraries (TARGET "${UNIT_NAME}" PRIVATE "${ABSTRACT_NAME}")
    target_compile_definitions ("${UNIT_NAME}" PRIVATE "${ABSTRACT_NAME}Implementation")
endfunction ()

# Starts configuration routine of abstract unit: headers that might have multiple implementations.
function (register_abstract UNIT_NAME)
    message (STATUS "Registering abstract \"${UNIT_NAME}\"...")
    add_library ("${UNIT_NAME}" INTERFACE)
    set_target_properties ("${UNIT_NAME}" PROPERTIES UNIT_TARGET_TYPE "Abstract")

    # Generate API header for shared library support.
    file (MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Generated")
    generate_api_header (
            API_MACRO "${UNIT_NAME}Api"
            EXPORT_MACRO "${UNIT_NAME}Implementation"
            OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/Generated/${UNIT_NAME}Api.hpp")
    target_include_directories ("${UNIT_NAME}" INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/Generated")

    set (UNIT_NAME "${UNIT_NAME}" PARENT_SCOPE)
endfunction ()

# Adds given directories to interface include list of current abstract unit.
function (abstract_include INCLUDE_DIR)
    # Technically, abstract is an advanced interface. Therefore we're using some of the interface functions.
    interface_include (${ARGV})
endfunction ()

# Registers requirements of current abstract unit.
# Arguments:
# - ABSTRACT: list of required abstract units.
# - CONCRETE_INTERFACE: list of required interfaces of concrete units.
# - INTERFACE: list of required interface units.
# - THIRD_PARTY: list of required third party targets.
function (abstract_require)
    # Technically, abstract is an advanced interface. Therefore we're using some of the interface functions.
    interface_require (${ARGV})
endfunction ()

define_property (TARGET PROPERTY IMPLEMENTATIONS
        BRIEF_DOCS "List of implementations of the abstract unit."
        FULL_DOCS "Saving list of implementations makes it easy to add custom logic to build system.")

define_property (TARGET PROPERTY IMPLEMENTATION_REMAP
        BRIEF_DOCS "List of concrete units that form implementation of associated abstract unit."
        FULL_DOCS "Added to special implementation marker target.")

# Registers new implementation for current abstract unit.
# Arguments:
# - NAME: name of the implementation.
# - PARTS: list of concrete units that form implementation of this abstract unit.
function (abstract_register_implementation)
    cmake_parse_arguments ("IMPLEMENTATION" "" "NAME" "PARTS" ${ARGN})
    if (DEFINED IMPLEMENTATION_UNPARSED_ARGUMENTS OR
            NOT DEFINED IMPLEMENTATION_NAME OR
            NOT DEFINED IMPLEMENTATION_PARTS)
        message (FATAL_ERROR "Incorrect function arguments!")
    endif ()

    message (STATUS "    Add implementation \"${IMPLEMENTATION_NAME}\".")
    add_library ("${UNIT_NAME}${IMPLEMENTATION_NAME}Marker" INTERFACE)

    foreach (PART_NAME ${IMPLEMENTATION_PARTS})
        message (STATUS "        Add part \"${PART_NAME}\".")
    endforeach ()

    set_target_properties ("${UNIT_NAME}${IMPLEMENTATION_NAME}Marker" PROPERTIES
            UNIT_TARGET_TYPE "AbstractImplementation"
            IMPLEMENTATION_REMAP "${IMPLEMENTATION_PARTS}")

    get_target_property (IMPLEMENTATIONS "${UNIT_NAME}" IMPLEMENTATIONS)
    if (IMPLEMENTATIONS STREQUAL "IMPLEMENTATIONS-NOTFOUND")
        set (IMPLEMENTATIONS)
    endif ()

    list (APPEND IMPLEMENTATIONS "${IMPLEMENTATION_NAME}")
    set_target_properties ("${UNIT_NAME}" PROPERTIES IMPLEMENTATIONS "${IMPLEMENTATIONS}")
endfunction ()

# Outputs list of all implementations of request abstract unit.
# Arguments:
# - ABSTRACT: name of the abstract unit.
# - OUTPUT: name of the output variable.
function (abstract_get_implementations)
    cmake_parse_arguments ("SEARCH" "" "ABSTRACT;OUTPUT" "" ${ARGV})
    if (DEFINED SEARCH_UNPARSED_ARGUMENTS OR
            NOT DEFINED SEARCH_ABSTRACT OR
            NOT DEFINED SEARCH_OUTPUT)
        message (FATAL_ERROR "Incorrect function arguments!")
    endif ()

    get_target_property (IMPLEMENTATIONS "${SEARCH_ABSTRACT}" IMPLEMENTATIONS)
    if (IMPLEMENTATIONS STREQUAL "IMPLEMENTATIONS-NOTFOUND")
        set ("${SEARCH_OUTPUT}" "" PARENT_SCOPE)
    else ()
        set ("${SEARCH_OUTPUT}" "${IMPLEMENTATIONS}" PARENT_SCOPE)
    endif ()
endfunction ()

# Starts configuration routine of shared library.
function (register_shared_library ARTEFACT_NAME)
    message (STATUS "Registering shared library \"${ARTEFACT_NAME}\"...")
    add_library ("${ARTEFACT_NAME}" SHARED)
    set (ARTEFACT_NAME "${ARTEFACT_NAME}" PARENT_SCOPE)

    # Force Windows-like behaviour on rpath-driven unix builds.
    if (UNIX)
        set_target_properties ("${ARTEFACT_NAME}" PROPERTIES BUILD_RPATH "\$ORIGIN")
    endif ()
endfunction ()

# Adds given units to current shared library content.
# Arguments:
# - SCOPE: Scope in which these units are being added, either PUBLIC or PRIVATE.
# - ABSTRACT: list of abstract units implementations that are being added to this library in format
#             "ABSTRACT_UNIT_NAME=IMPLEMENTATION_NAME".
# - CONCRETE: list of concrete units that are being added to this library.
function (shared_library_include)
    cmake_parse_arguments (INCLUDE "" "SCOPE" "ABSTRACT;CONCRETE" ${ARGV})
    if (DEFINED INCLUDE_UNPARSED_ARGUMENTS OR
            NOT DEFINED INCLUDE_SCOPE OR (
            NOT DEFINED INCLUDE_ABSTRACT AND
            NOT DEFINED INCLUDE_CONCRETE))
        message (FATAL_ERROR "Incorrect function arguments!")
    endif ()

    if (INCLUDE_SCOPE STREQUAL "PUBLIC")
        set (SCOPE_STRING "public")
    elseif (INCLUDE_SCOPE STREQUAL "PRIVATE")
        set (SCOPE_STRING "private")
    else ()
        message (FATAL_ERROR "Incorrect function arguments!")
    endif ()

    if (DEFINED INCLUDE_ABSTRACT)
        foreach (SELECTION ${INCLUDE_ABSTRACT})
            if (NOT SELECTION MATCHES "^([A-Za-z0-9_]+)=([A-Za-z0-9_]+)$")
                message (SEND_ERROR "Unable to parse abstract selection \"${SELECTION}\".")
            else ()

                set (ABSTRACT_NAME "${CMAKE_MATCH_1}")
                set (IMPLEMENTATION_NAME "${CMAKE_MATCH_2}")
                message (STATUS "    Add ${SCOPE_STRING} implementation \"${IMPLEMENTATION_NAME}\" of abstract \"${ABSTRACT_NAME}\".")

                if (TARGET "${ABSTRACT_NAME}${IMPLEMENTATION_NAME}Marker")
                    get_target_property (REMAP "${ABSTRACT_NAME}${IMPLEMENTATION_NAME}Marker" IMPLEMENTATION_REMAP)

                    if (REMAP STREQUAL "REMAP-NOTFOUND")
                        message (SEND_ERROR "Abstract \"${ABSTRACT_NAME}\" implementation \"${IMPLEMENTATION_NAME}\" is empty!")
                    else ()
                        reflected_target_link_libraries (
                                TARGET "${ARTEFACT_NAME}"
                                ${INCLUDE_SCOPE} "${ABSTRACT_NAME}${IMPLEMENTATION_NAME}Marker")

                        foreach (PART_NAME ${REMAP})
                            message (STATUS "        Include part \"${PART_NAME}\".")
                            reflected_target_link_libraries (TARGET "${ARTEFACT_NAME}" ${INCLUDE_SCOPE} "${PART_NAME}")
                        endforeach ()
                    endif ()

                else ()
                    message (SEND_ERROR "Abstract \"${ABSTRACT_NAME}\" implementation \"${IMPLEMENTATION_NAME}\" can only be included after it is registered!")
                endif ()

            endif ()
        endforeach ()
    endif ()

    if (DEFINED INCLUDE_CONCRETE)
        foreach (DEPENDENCY ${INCLUDE_CONCRETE})
            message (STATUS "    Add ${SCOPE_STRING} scope concrete \"${DEPENDENCY}\".")
            reflected_target_link_libraries (TARGET "${ARTEFACT_NAME}" ${INCLUDE_SCOPE} "${DEPENDENCY}")
        endforeach ()
    endif ()

endfunction ()

# Links other shared libraries to current shared library.
# Arguments:
# - PRIVATE: list of shared libraries to be linked in private scope.
# - PUBLIC: list of shared libraries to be linked in public scope.
function (shared_library_link_shared_library)
    cmake_parse_arguments (LINK "" "" "PRIVATE;PUBLIC" ${ARGV})
    if (DEFINED LINK_UNPARSED_ARGUMENTS OR
            NOT DEFINED LINK_PRIVATE AND
            NOT DEFINED LINK_PUBLIC)
        message (FATAL_ERROR "Incorrect function arguments!")
    endif ()

    foreach (LIBRARY_TARGET ${LINK_PRIVATE})
        message (STATUS "    Link shared library \"${LIBRARY_TARGET}\" in private scope.")
        reflected_target_link_libraries (TARGET "${ARTEFACT_NAME}" PRIVATE "${LIBRARY_TARGET}")
    endforeach ()

    foreach (LIBRARY_TARGET ${LINK_PUBLIC})
        message (STATUS "    Link shared library \"${LIBRARY_TARGET}\" in public scope.")
        reflected_target_link_libraries (TARGET "${ARTEFACT_NAME}" PUBLIC "${LIBRARY_TARGET}")
    endforeach ()
endfunction ()

# Verifies that there is no missing abstract unit implementations or concrete units in current shared library.
function (shared_library_verify)
    message (STATUS "    Verifying...")
    find_linked_targets_recursively (TARGET "${ARTEFACT_NAME}" OUTPUT ALL_LOCAL_TARGETS ARTEFACT_SCOPE)
    find_linked_targets_recursively (TARGET "${ARTEFACT_NAME}" OUTPUT ALL_VISIBLE_TARGETS CHECK_VISIBILITY)

    foreach (LOCAL_TARGET ${ALL_LOCAL_TARGETS})
        get_target_property (TARGET_TYPE "${LOCAL_TARGET}" UNIT_TARGET_TYPE)
        if (TARGET_TYPE STREQUAL "ConcreteInterface")

            get_target_property (CONCRETE_REQUIREMENT "${LOCAL_TARGET}" REQUIRED_CONCRETE_UNIT)
            list (FIND ALL_VISIBLE_TARGETS "${CONCRETE_REQUIREMENT}" REQUIREMENT_INDEX)

            if (REQUIREMENT_INDEX EQUAL -1)
                message (SEND_ERROR "Target \"${ARTEFACT_NAME}\": Missing \"${CONCRETE_REQUIREMENT}\". Found interface, but no implementation.")
            endif ()

        elseif (TARGET_TYPE STREQUAL "Abstract")

            get_target_property (IMPLEMENTATIONS "${LOCAL_TARGET}" IMPLEMENTATIONS)
            set (FOUND_IMPLEMENTATION OFF)

            foreach (IMPLEMENTATION ${IMPLEMENTATIONS})
                list (FIND ALL_VISIBLE_TARGETS "${LOCAL_TARGET}${IMPLEMENTATION}Marker" IMPLEMENTATION_INDEX)
                if (NOT IMPLEMENTATION_INDEX EQUAL -1)
                    if (FOUND_IMPLEMENTATION)
                        message (SEND_ERROR "Target \"${ARTEFACT_NAME}\": Found multiple implementations of abstract \"${LOCAL_TARGET}\".")
                    else ()
                        set (FOUND_IMPLEMENTATION ON)
                    endif ()
                endif ()
            endforeach ()

            if (NOT FOUND_IMPLEMENTATION)
                message (SEND_ERROR "Target \"${ARTEFACT_NAME}\": Missing abstract \"${LOCAL_TARGET}\" implementation.")
            endif ()

        endif ()
    endforeach ()
endfunction ()

# Starts configuration routine of executable.
function (register_executable ARTEFACT_NAME)
    message (STATUS "Registering executable \"${ARTEFACT_NAME}\"...")
    add_executable ("${ARTEFACT_NAME}")
    set (ARTEFACT_NAME "${ARTEFACT_NAME}" PARENT_SCOPE)

    # Force Windows-like behaviour on rpath-driven unix builds.
    if (UNIX)
        set_target_properties ("${ARTEFACT_NAME}" PROPERTIES BUILD_RPATH "\$ORIGIN")
    endif ()
endfunction ()

# Adds given units to current executable content.
# Arguments:
# - ABSTRACT: list of abstract units implementations that are being added to this executable in format
#             "ABSTRACT_UNIT_NAME=IMPLEMENTATION_NAME".
# - CONCRETE: list of concrete units that are being added to this executable.
function (executable_include)
    # Technically, we're doing the same thing except for the scope, so it is ok to call shared library function.
    shared_library_include (SCOPE PRIVATE ${ARGV})
endfunction ()

# Links given shared libraries to current executable.
function (executable_link_shared_libraries)
    foreach (LIBRARY_TARGET ${ARGV})
        message (STATUS "    Link shared library \"${LIBRARY_TARGET}\".")
        reflected_target_link_libraries (TARGET "${ARTEFACT_NAME}" PRIVATE "${LIBRARY_TARGET}")
    endforeach ()
endfunction ()

# Verifies that there is no missing abstract unit implementations or concrete units in current executable.
function (executable_verify)
    # Technically, we're doing the same thing except for the scope, so it is ok to call shared library function.
    shared_library_verify ()
endfunction ()

# Adds build commands for copying required linked artefacts to current executable.
function (executable_copy_linked_artefacts)
    find_linked_shared_libraries (TARGET "${ARTEFACT_NAME}" OUTPUT REQUIRED_LIBRARIES)
    foreach (LIBRARY_TARGET ${REQUIRED_LIBRARIES})
        if (UNIX)
            add_custom_command (
                    TARGET "${ARTEFACT_NAME}" POST_BUILD
                    COMMAND
                    ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_SONAME_FILE:${LIBRARY_TARGET}> $<TARGET_FILE_DIR:${ARTEFACT_NAME}>
                    COMMAND_EXPAND_LISTS)
        else ()
            add_custom_command (
                    TARGET "${ARTEFACT_NAME}" POST_BUILD
                    COMMAND
                    ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE:${LIBRARY_TARGET}> $<TARGET_FILE_DIR:${ARTEFACT_NAME}>
                    COMMAND_EXPAND_LISTS)
        endif ()
    endforeach ()
endfunction ()
