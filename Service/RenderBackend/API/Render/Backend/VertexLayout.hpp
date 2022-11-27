#pragma once

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Shortcuts.hpp>

namespace Emergence::Render::Backend
{
enum class Attribute : uint8_t
{
    POSITION,
    NORMAL,
    TANGENT,
    BITANGENT,
    COLOR_0,
    COLOR_1,
    COLOR_2,
    COLOR_3,
    INDICES,
    WEIGHT,
    SAMPLER_COORD_0,
    SAMPLER_COORD_1,
    SAMPLER_COORD_2,
    SAMPLER_COORD_3,
    SAMPLER_COORD_4,
    SAMPLER_COORD_5,
    SAMPLER_COORD_6,
    SAMPLER_COORD_7,
};

enum class AttributeType : uint8_t
{
    INT8 = 0u,
    INT16,
    HALF_FLOAT,
    FLOAT
};

class VertexLayout final
{
public:
    VertexLayout () noexcept;

    VertexLayout (const VertexLayout &_other) = delete;

    VertexLayout (VertexLayout &&_other) noexcept;

    ~VertexLayout () noexcept;

    VertexLayout &operator= (const VertexLayout &_other) = delete;

    VertexLayout &operator= (VertexLayout &&_other) noexcept;

private:
    friend class VertexLayoutBuilder;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t) * 11u);
};

class VertexLayoutBuilder final
{
public:
    VertexLayoutBuilder () noexcept;

    VertexLayoutBuilder (const VertexLayoutBuilder &_other) = delete;

    VertexLayoutBuilder (VertexLayoutBuilder &&_other) = delete;

    ~VertexLayoutBuilder () noexcept;

    void Begin () noexcept;

    void Add (Attribute _attribute, AttributeType _type, uint16_t _elementCount) noexcept;

    VertexLayout End () noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (VertexLayoutBuilder);

private:
    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t) * 11u);
};
} // namespace Emergence::Render::Backend
