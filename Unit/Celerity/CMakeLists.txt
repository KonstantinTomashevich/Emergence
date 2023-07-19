call_code_generation (${CMAKE_CURRENT_SOURCE_DIR})
set (CELERITY_PROFILE "Standard" CACHE STRING "Name of constants profile for Celerity unit.")
register_concrete (Celerity)
concrete_include (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Constants" "${CMAKE_CURRENT_SOURCE_DIR}/Public")

concrete_sources ("Public/*.cpp")
concrete_require (SCOPE PRIVATE ABSTRACT Log CONCRETE_INTERFACE Time)
concrete_require (
        SCOPE PUBLIC
        ABSTRACT CPUProfiler TaskExecutor Warehouse
        CONCRETE_INTERFACE Flow Handling Math)

if (CELERITY_PROFILE STREQUAL "Standard")
    concrete_include (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/ConstantsStandard")
else ()
    message (SEND_ERROR "Celerity: Unknown profile \"${CELERITY_PROFILE}\".")
endif ()
