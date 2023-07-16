set (FLOW_PROFILE "Standard" CACHE STRING "Name of constants profile for Flow unit.")
register_concrete (Flow)
concrete_include (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Constants" "${CMAKE_CURRENT_SOURCE_DIR}/Public")

concrete_sources ("Public/*.cpp")
concrete_require (SCOPE PRIVATE ABSTRACT Log)

concrete_require (
        SCOPE PUBLIC
        CONCRETE_INTERFACE TaskCollection VisualGraph
        INTERFACE APICommon)

if (FLOW_PROFILE STREQUAL "Standard")
    concrete_include (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/ConstantsStandard")
else ()
    message (SEND_ERROR "Flow: Unknown profile \"${FLOW_PROFILE}\".")
endif ()
