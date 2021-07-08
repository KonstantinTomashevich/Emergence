# TODO: It could make sense to create unit testing service, but for now I decided to just use Boost.Test for tests.
# TODO: Try https://github.com/onqtam/doctest, it should compile faster than Boost.
find_package (Boost COMPONENTS unit_test_framework REQUIRED)

enable_testing ()
add_subdirectory (Library)
add_subdirectory (Service)