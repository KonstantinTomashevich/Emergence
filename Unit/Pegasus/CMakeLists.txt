set (PEGASUS_PROFILE "Standard" CACHE STRING "Name of constants profile for Pegasus unit.")
register_concrete (Pegasus)

concrete_include (
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Private"
        PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Constants" "${CMAKE_CURRENT_SOURCE_DIR}/Public")

concrete_sources ("Public/*.cpp" "Private/*.cpp")
concrete_require (SCOPE PRIVATE ABSTRACT Hashing)

concrete_require (
        SCOPE PUBLIC
        ABSTRACT StandardLayoutMapping Memory MemoryProfiler
        CONCRETE_INTERFACE Container Handling SyntaxSugar
        INTERFACE APICommon)

if (PEGASUS_PROFILE STREQUAL "Standard")
    concrete_include (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/ConstantsStandard")
else ()
    message (SEND_ERROR "Pegasus: Unknown profile \"${PEGASUS_PROFILE}\".")
endif ()
