register_concrete (Serialization)
concrete_include (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")
concrete_require (SCOPE PRIVATE ABSTRACT Log THIRD_PARTY yaml-cpp)
concrete_require (SCOPE PUBLIC ABSTRACT StandardLayoutMapping)

# On MSVC CLang exceptions are disabled by default,
# but we need to compile with exceptions because of yaml-cpp templates.
if (MSVC)
    concrete_compile_options (PRIVATE /EHa)
endif ()
