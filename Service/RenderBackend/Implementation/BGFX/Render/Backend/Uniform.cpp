#include <Assert/Assert.hpp>

#include <bgfx/bgfx.h>

#include <Render/Backend/Uniform.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::Render::Backend
{
static bgfx::UniformType::Enum ToBGFX (UniformType _type)
{
    switch (_type)
    {
    case UniformType::VECTOR_4F:
        return bgfx::UniformType::Vec4;

    case UniformType::MATRIX_3X3F:
        return bgfx::UniformType::Mat3;

    case UniformType::MATRIX_4X4F:
        return bgfx::UniformType::Mat4;

    case UniformType::SAMPLER:
        return bgfx::UniformType::Sampler;
    }

    return bgfx::UniformType::Count;
}

Uniform::Uniform () noexcept
{
    block_cast<uint16_t> (data) = bgfx::kInvalidHandle;
}

Uniform::Uniform (Memory::UniqueString _name, UniformType _type) noexcept
{
    block_cast<uint16_t> (data) = bgfx::createUniform (*_name, ToBGFX (_type)).idx;
}

Uniform::Uniform (Uniform &&_other) noexcept
{
    data = _other.data;
    block_cast<uint16_t> (_other.data) = bgfx::kInvalidHandle;
}

Uniform::~Uniform () noexcept
{
    if (uint16_t handle = block_cast<uint16_t> (data); handle != bgfx::kInvalidHandle)
    {
        bgfx::destroy (bgfx::UniformHandle {handle});
    }
}

void Uniform::SetVector4f (const Math::Vector4f &_value) noexcept
{
    uint16_t handle = block_cast<uint16_t> (data);
    EMERGENCE_ASSERT (handle != bgfx::kInvalidHandle);
    bgfx::setUniform (bgfx::UniformHandle {handle}, _value.components);
}

void Uniform::SetMatrix3x3f (const Math::Matrix3x3f &_value) noexcept
{
    uint16_t handle = block_cast<uint16_t> (data);
    EMERGENCE_ASSERT (handle != bgfx::kInvalidHandle);
    bgfx::setUniform (bgfx::UniformHandle {handle}, _value.columns);
}

void Uniform::SetMatrix4x4f (const Math::Matrix4x4f &_value) noexcept
{
    uint16_t handle = block_cast<uint16_t> (data);
    EMERGENCE_ASSERT (handle != bgfx::kInvalidHandle);
    bgfx::setUniform (bgfx::UniformHandle {handle}, _value.columns);
}

void Uniform::SetSampler (uint8_t _stage, TextureId _texture) noexcept
{
    uint16_t handle = block_cast<uint16_t> (data);
    EMERGENCE_ASSERT (handle != bgfx::kInvalidHandle);
    bgfx::setTexture (_stage, bgfx::UniformHandle {handle}, bgfx::TextureHandle {static_cast<uint16_t> (_texture)});
}

bool Uniform::IsValid () const noexcept
{
    return block_cast<uint16_t> (data) != bgfx::kInvalidHandle;
}

Uniform &Uniform::operator= (Uniform &&_other) noexcept
{
    if (this != &_other)
    {
        this->~Uniform ();
        new (this) Uniform (std::move (_other));
    }

    return *this;
}
} // namespace Emergence::Render::Backend
