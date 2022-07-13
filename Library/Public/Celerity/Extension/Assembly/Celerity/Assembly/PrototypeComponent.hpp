#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct PrototypeComponent final
{
    UniqueId objectId = INVALID_UNIQUE_ID;
    Memory::UniqueString descriptorId;

    struct Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId descriptorId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
