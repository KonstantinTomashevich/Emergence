#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <Math/Vector2f.hpp>

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct Camera2dComponent final
{
    UniqueId objectId = INVALID_UNIQUE_ID;
    float halfOrthographicSize = 0.0f;

    struct Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId halfOrthographicSize;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
