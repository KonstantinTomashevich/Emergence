#pragma once

#include <cstdint>

#include <API/Common/ImplementationBinding.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::StandardLayout
{
/// \brief Contains set of changes that transform object from one state to another.
///
/// \details PatchBuilder should be used to construct patches.
class Patch final
{
public:
    Patch (const Patch &_other) noexcept;

    Patch (Patch &&_other) noexcept;

    ~Patch () noexcept;

    /// \return Type of objects to which this patch can be applied.
    [[nodiscard]] Mapping GetTypeMapping () const noexcept;

    /// \brief Applies transformation, defined by patch, to given object.
    /// \details If object state is different from the initial state for which patch was created,
    ///          transformation result would be different too.
    /// \invariant Object belongs to ::GetTypeMapping type.
    void Apply (void *_object) const noexcept;

    /// Assigning patches looks counter-intuitive.
    EMERGENCE_DELETE_ASSIGNMENT (Patch);

private:
    friend class PatchBuilder;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));

    /// \brief Copies implementation-specific values from given pointer.
    explicit Patch (const std::array<uint8_t, DATA_MAX_SIZE> &_data) noexcept;

    /// \brief Moves implementation-specific values from given pointer.
    explicit Patch (std::array<uint8_t, DATA_MAX_SIZE> &_data) noexcept;
};
} // namespace Emergence::StandardLayout
