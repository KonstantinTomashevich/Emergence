#include <Resource/Provider/Test/ResourceProvider.hpp>

#include <Testing/SetupMain.hpp>

// Use "include marker", so linker will not omit translation units with autoregistered tests.
static const bool RESOURCE_PROVIDER_TEST_INCLUDE_MARKER =
    Emergence::Resource::Provider::Test::ResourceProviderTestIncludeMarker();
