#pragma once

#include <ResourceProvider/ResourceProvider.hpp>

namespace Emergence::Celerity::Test
{
struct ResourceProviderHolder
{
    ResourceProviderHolder () noexcept;

    ResourceProvider::ResourceProvider provider;
};

ResourceProvider::ResourceProvider &GetSharedResourceProvider () noexcept;
} // namespace Emergence::Celerity::Test
