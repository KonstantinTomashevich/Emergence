#pragma once

#include <cstdint>

#include <StandardLayout/Field.hpp>

namespace Emergence::StandardLayout
{
class Mapping final
{
public:
    Mapping (const Mapping &_other) noexcept;

    Mapping (Mapping &&other) noexcept;

    std::size_t GetObjectSize () const noexcept;

    const FieldMeta &GetField (FieldId _field) const noexcept;

private:
    friend class MappingBuilder;

    Mapping (std::size_t _objectSize) noexcept;

    ~Mapping () noexcept;

    /// \brief Max total size of mapping type fields in bytes.
    ///
    /// \details It could be critical for performance to store mapping instance, not handle, as field of other class.
    /// Therefore we specify max total size of fields for any implementation and reserve this space using std::array.
    static constexpr std::size_t MAX_FIELDS_SIZE = sizeof (uintptr_t) * 5u;

    /// \brief Stub, that reserves space for implementation fields.
    /// \see ::MAX_FIELDS_SIZE.
    std::array <uint8_t, MAX_FIELDS_SIZE> fields;
};
} // namespace Emergence::StandardLayout
