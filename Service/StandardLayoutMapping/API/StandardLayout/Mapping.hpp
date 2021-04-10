#pragma once

#include <cstdint>

#include <StandardLayout/Field.hpp>

namespace Emergence::StandardLayout
{
/// \brief Finished field mapping for user defined object type.
///
/// \brief MappingBuilder should be used to construct mappings.
class Mapping final
{
public:
    Mapping (const Mapping &_other) noexcept;

    Mapping (Mapping &&other) noexcept;

    /// \return user defined object size in bytes including alignment gaps.
    std::size_t GetObjectSize () const noexcept;

    /// \return pointer to meta of field with given id or `nullptr` if there is no such field.
    const FieldMeta *const GetField (FieldId _field) const noexcept;

private:
    friend class MappingBuilder;

    Mapping (std::size_t _objectSize) noexcept;

    ~Mapping () noexcept;

    /// \brief Mapping implementation handle.
    void *handle;
};
} // namespace Emergence::StandardLayout
