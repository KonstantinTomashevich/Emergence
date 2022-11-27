#pragma once

#include <Math/Matrix3x3f.hpp>
#include <Math/Matrix4x4f.hpp>
#include <Math/Vector4f.hpp>

#include <Memory/UniqueString.hpp>

#include <Render/Backend/Texture.hpp>

namespace Emergence::Render::Backend
{
/// \brief Defines supported uniform types.
enum class UniformType : uint8_t
{
    /// \brief 4 dimensional vector.
    VECTOR_4F = 0u,

    /// \brief 3x3 sized matrix.
    MATRIX_3X3F,

    /// \brief 4x4 sized matrix.
    MATRIX_4X4F,

    /// \brief 2d texture sampler.
    SAMPLER,
};

class Uniform final
{
public:
    Uniform (Memory::UniqueString _name, UniformType _type) noexcept;

    Uniform (const Uniform &_other) = delete;

    Uniform (Uniform &&_other) noexcept;

    ~Uniform () noexcept;

    void SetVector4f (const Math::Vector4f &_value) noexcept;

    void SetMatrix3x3f (const Math::Matrix3x3f &_value) noexcept;

    void SetMatrix4x4f (const Math::Matrix4x4f &_value) noexcept;

    void SetSampler (uint16_t _stage, const Texture &_texture) noexcept;

    Uniform &operator= (const Uniform &_other) = delete;

    Uniform &operator= (Uniform &&_other) noexcept;

private:
    EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();
};
} // namespace Emergence::Render::Backend
