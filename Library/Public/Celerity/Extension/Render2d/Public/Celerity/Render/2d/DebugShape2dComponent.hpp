#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <Math/Vector2f.hpp>

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Enumerates types of supported shapes for debug drawing.
enum class DebugShape2dType : uint8_t
{
    BOX = 0u,
    CIRCLE,
    LINE,
};

/// \brief Describes shape that can be used to draw 2d debug information on screen.
struct DebugShape2d final
{
    /// \brief Constructs new DebugShape2dType::BOX shape.
    DebugShape2d (float _boxHalfWidth, float _boxHalfHeight) noexcept;

    /// \brief Constructs new DebugShape2dType::CIRCLE shape.
    DebugShape2d (float _circleRadius) noexcept;

    /// \brief Constructs new DebugShape2dType::LINE shape.
    DebugShape2d (const Math::Vector2f &_lineEnd) noexcept;

    /// \brief Type of the shape that is used for debug rendering.
    DebugShape2dType type;

    union
    {
        /// \brief Half extents of the box for DebugShape2dType::BOX.
        Math::Vector2f boxHalfExtents;

        /// \brief Circle radius for DebugShape2dType::CIRCLE.
        float circleRadius;

        /// \brief Line end point for DebugShape2dType::LINE. Start point is always (0, 0).
        Math::Vector2f lineEnd;
    };

    struct Reflection final
    {
        StandardLayout::FieldId type;
        StandardLayout::FieldId boxHalfExtents;
        StandardLayout::FieldId circleRadius;
        StandardLayout::FieldId lineEnd;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

/// \brief Attaches debug draw feature to an object.
struct DebugShape2dComponent final
{
    /// \brief Id of an object with Transform2dComponent to which this debug draw shape is attached.
    UniqueId objectId = INVALID_UNIQUE_ID;

    /// \brief Unique id of this debug draw shape.
    /// \invariant Must be generated using Render2dSingleton::GenerateDebugShape2dId.
    UniqueId debugShapeId = INVALID_UNIQUE_ID;

    /// \brief Material instance that should be used to render the debug draw shape.
    Memory::UniqueString materialInstanceId;

    /// \brief Shape additional translation, local to object transform.
    Math::Vector2f translation = Math::Vector2f::ZERO;

    /// \brief Shape additional rotation, local to object transform.
    float rotation = 0.0f;

    /// \brief Debug draw shape to be rendered.
    DebugShape2d shape;

    struct Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId debugShapeId;
        StandardLayout::FieldId materialInstanceId;
        StandardLayout::FieldId translation;
        StandardLayout::FieldId rotation;
        StandardLayout::FieldId shape;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
