add_subdirectory (doctest)

# We don't need exceptions neither from spdlog nor fmt.
add_compile_definitions (FMT_EXCEPTIONS=0)
set (SPDLOG_NO_EXCEPTIONS ON)
add_subdirectory (spdlog)

add_subdirectory (xxhash_cpp)
