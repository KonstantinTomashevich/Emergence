#pragma once

#include <cstdint>

#include <StandardLayout/Field.hpp>

namespace Emergence::StandardLayout
{
/// \brief Finished field mapping for user defined object type.
///
/// \details MappingBuilder should be used to construct mappings.
class Mapping final
{
public:
    Mapping (const Mapping &_other) noexcept;

    Mapping (Mapping &&_other) noexcept;

    /// \return user defined object size in bytes including alignment gaps.
    std::size_t GetObjectSize () const noexcept;

    /// \return pointer to meta of field with given id or `nullptr` if there is no such field.
    Field GetField (FieldId _field) const noexcept;

    // TODO: Add field list getter? Is it really required?

private:
    /// Mapping builder constructs mappings.
    friend class MappingBuilder;

    /// Field::GetInstanceMapping() wraps implementation data into Mapping interface.
    friend class Field;

    explicit Mapping (void *_handle) noexcept;

    ~Mapping () noexcept;

    /// \brief Mapping implementation handle.
    void *handle = nullptr;
};
} // namespace Emergence::StandardLayout
