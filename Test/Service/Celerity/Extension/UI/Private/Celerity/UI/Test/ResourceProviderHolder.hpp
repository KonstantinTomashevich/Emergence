#pragma once

#include <Resource/Provider/ResourceProvider.hpp>

namespace Emergence::Celerity::Test
{
struct ResourceProviderHolder
{
    ResourceProviderHolder () noexcept;

    Resource::Provider::ResourceProvider provider;
};

Resource::Provider::ResourceProvider &GetSharedResourceProvider () noexcept;
} // namespace Emergence::Celerity::Test
