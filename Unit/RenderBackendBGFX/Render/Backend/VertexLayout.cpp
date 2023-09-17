#include <API/Common/BlockCast.hpp>

#include <bgfx/bgfx.h>

#include <Render/Backend/VertexLayout.hpp>

namespace Emergence::Render::Backend
{
static bgfx::Attrib::Enum ToBGFX (Attribute _attribute)
{
    switch (_attribute)
    {
    case Attribute::POSITION:
        return bgfx::Attrib::Position;

    case Attribute::NORMAL:
        return bgfx::Attrib::Normal;

    case Attribute::TANGENT:
        return bgfx::Attrib::Tangent;

    case Attribute::BITANGENT:
        return bgfx::Attrib::Bitangent;

    case Attribute::COLOR_0:
        return bgfx::Attrib::Color0;

    case Attribute::COLOR_1:
        return bgfx::Attrib::Color1;

    case Attribute::COLOR_2:
        return bgfx::Attrib::Color2;

    case Attribute::COLOR_3:
        return bgfx::Attrib::Color3;

    case Attribute::INDICES:
        return bgfx::Attrib::Indices;

    case Attribute::WEIGHT:
        return bgfx::Attrib::Weight;

    case Attribute::SAMPLER_COORD_0:
        return bgfx::Attrib::TexCoord0;

    case Attribute::SAMPLER_COORD_1:
        return bgfx::Attrib::TexCoord1;

    case Attribute::SAMPLER_COORD_2:
        return bgfx::Attrib::TexCoord2;

    case Attribute::SAMPLER_COORD_3:
        return bgfx::Attrib::TexCoord3;

    case Attribute::SAMPLER_COORD_4:
        return bgfx::Attrib::TexCoord4;

    case Attribute::SAMPLER_COORD_5:
        return bgfx::Attrib::TexCoord5;

    case Attribute::SAMPLER_COORD_6:
        return bgfx::Attrib::TexCoord6;

    case Attribute::SAMPLER_COORD_7:
        return bgfx::Attrib::TexCoord7;
    }

    return bgfx::Attrib::Count;
}

static bgfx::AttribType::Enum ToBGFX (AttributeType _attributeType)
{
    switch (_attributeType)
    {
    case AttributeType::UINT8:
        return bgfx::AttribType::Uint8;

    case AttributeType::INT16:
        return bgfx::AttribType::Int16;

    case AttributeType::HALF_FLOAT:
        return bgfx::AttribType::Half;

    case AttributeType::FLOAT:
        return bgfx::AttribType::Float;
    }

    return bgfx::AttribType::Count;
}

VertexLayout::VertexLayout (VertexLayout &&_other) noexcept
{
    new (data.data ()) bgfx::VertexLayout (block_cast<bgfx::VertexLayout> (_other.data));
}

VertexLayout::~VertexLayout () noexcept
{
    block_cast<bgfx::VertexLayout> (data).~VertexLayout ();
}

VertexLayout &VertexLayout::operator= (VertexLayout &&_other) noexcept
{
    if (this != &_other)
    {
        this->~VertexLayout ();
        new (this) VertexLayout (std::move (_other));
    }

    return *this;
}

VertexLayout::VertexLayout (std::array<std::uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    new (data.data ()) bgfx::VertexLayout (block_cast<bgfx::VertexLayout> (*_data));
}

VertexLayoutBuilder::VertexLayoutBuilder () noexcept = default;

VertexLayoutBuilder::~VertexLayoutBuilder () noexcept = default;

VertexLayoutBuilder &VertexLayoutBuilder::Begin () noexcept
{
    auto *layout = new (data.data ()) bgfx::VertexLayout ();
    layout->begin (bgfx::getRendererType ());
    return *this;
}

VertexLayoutBuilder &VertexLayoutBuilder::Add (Attribute _attribute,
                                               AttributeType _type,
                                               std::uint8_t _elementCount,
                                               bool _normalized) noexcept
{
    block_cast<bgfx::VertexLayout> (data).add (ToBGFX (_attribute), _elementCount, ToBGFX (_type), _normalized);
    return *this;
}

VertexLayout VertexLayoutBuilder::End () noexcept
{
    auto &layout = block_cast<bgfx::VertexLayout> (data);
    layout.end ();
    VertexLayout result {reinterpret_cast<decltype (VertexLayout::data) *> (&layout)};
    layout.~VertexLayout ();
    return result;
}
} // namespace Emergence::Render::Backend
