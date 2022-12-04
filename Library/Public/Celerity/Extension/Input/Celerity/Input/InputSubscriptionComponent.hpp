#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct InputSubscriptionComponent final
{
    UniqueId objectId = INVALID_UNIQUE_ID;

    Memory::UniqueString groupId;

    struct Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId groupId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
