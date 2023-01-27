#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <Math/AxisAlignedBox2d.hpp>

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Represent two dimensional sprite that is attached to the world object.
/// \invariant Object must have Transform2dComponent in order to position the sprite.
struct Sprite2dComponent final
{
    /// \brief Id of an object with Transform2dComponent to which this sprite is attached.
    UniqueId objectId = INVALID_UNIQUE_ID;

    /// \brief Unique id of this sprite.
    /// \invariant Must be generated using Render2dSingleton::GenerateSprite2dId.
    UniqueId spriteId = INVALID_UNIQUE_ID;

    /// \brief Material instance that should be used to render the sprite.
    Memory::UniqueString materialInstanceId;

    /// \brief Sprite UV-mapping used for texture projection.
    Math::AxisAlignedBox2d uv;

    /// \brief Half size of the sprite.
    /// \details We expect half size in order to make geometry generation during rendering a little bit faster.
    Math::Vector2f halfSize;

    /// \brief Sprites are sorted by their layer.
    /// \details Sprites with higher value appear on top of the sprites with lower value.
    ///          It is advised to use the minimum possible amount of layers as sprites
    ///          from the different layers cannot be batched together.
    uint16_t layer = 0u;

    /// \brief Visibility mask used for filtering out unwanted drawables. See Camera2dComponent::visibilityMask.
    std::uint64_t visibilityMask = ~0u;

    struct Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId spriteId;
        StandardLayout::FieldId materialInstanceId;
        StandardLayout::FieldId uv;
        StandardLayout::FieldId halfSize;
        StandardLayout::FieldId layer;
        StandardLayout::FieldId visibilityMask;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
