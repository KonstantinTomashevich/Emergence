# TODO: Remove Boost dependency after migration to doctest.
find_package (Boost COMPONENTS unit_test_framework REQUIRED)

enable_testing ()
add_subdirectory (Library)
add_subdirectory (Service)