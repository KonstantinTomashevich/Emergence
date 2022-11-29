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

    /// \brief Texture sampler.
    SAMPLER,
};

/// \brief Represents named parameter that can be passed to GPU programs.
class Uniform final
{
public:
    /// \brief Constructs default invalid instance.
    Uniform () noexcept;

    /// \brief Registers parameter of given type with given name.
    Uniform (Memory::UniqueString _name, UniformType _type) noexcept;

    Uniform (const Uniform &_other) = delete;

    Uniform (Uniform &&_other) noexcept;

    ~Uniform () noexcept;

    /// \brief Sets given value to uniform.
    /// \invariant Uniform was constructed with UniformType::VECTOR_4F.
    void SetVector4f (const Math::Vector4f &_value) noexcept;

    /// \brief Sets given value to uniform.
    /// \invariant Uniform was constructed with UniformType::MATRIX_3X3F.
    void SetMatrix3x3f (const Math::Matrix3x3f &_value) noexcept;

    /// \brief Sets given value to uniform.
    /// \invariant Uniform was constructed with UniformType::MATRIX_4X4F.
    void SetMatrix4x4f (const Math::Matrix4x4f &_value) noexcept;

    /// \brief Submits given texture to given stage.
    /// \invariant Uniform was constructed with UniformType::SAMPLER.
    void SetSampler (uint8_t _stage, TextureId _texture) noexcept;

    [[nodiscard]] bool IsValid () const noexcept;

    Uniform &operator= (const Uniform &_other) = delete;

    Uniform &operator= (Uniform &&_other) noexcept;

private:
    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uint64_t));
};
} // namespace Emergence::Render::Backend
