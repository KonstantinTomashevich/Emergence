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
    std::uint64_t visibilityMask = ~0u;

    struct Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId halfOrthographicSize;
        StandardLayout::FieldId visibilityMask;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
