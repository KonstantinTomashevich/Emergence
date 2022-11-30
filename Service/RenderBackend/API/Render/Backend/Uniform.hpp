#pragma once

#include <Memory/UniqueString.hpp>

#include <Render/Backend/Texture.hpp>

namespace Emergence::Render::Backend
{
/// \brief Unique identifier used to reference existing Uniform.
using UniformId = std::uint64_t;

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

    /// \brief Unique identifier of this uniform.
    [[nodiscard]] UniformId GetId () const noexcept;

    [[nodiscard]] bool IsValid () const noexcept;

    Uniform &operator= (const Uniform &_other) = delete;

    Uniform &operator= (Uniform &&_other) noexcept;

private:
    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uint64_t));
};
} // namespace Emergence::Render::Backend
