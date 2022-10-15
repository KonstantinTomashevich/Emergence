#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <Math/Vector2f.hpp>

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct Camera2d final
{
    UniqueId objectId = INVALID_UNIQUE_ID;
    Math::Vector2f size;

    struct Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId size;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
