#pragma once

#include <Container/String.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct ConfigLoadingRequest final
{
    StandardLayout::Mapping type;

    bool forceReload = false;

    struct Reflection final
    {
        StandardLayout::FieldId forceReload;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct ConfigLoadingFinishedResponse final
{
    StandardLayout::Mapping type;

    struct Reflection final
    {
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
