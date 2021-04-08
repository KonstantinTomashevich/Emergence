#pragma once

#include <cstdint>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::StandardLayout
{
class MappingBuilder final
{
public:
    MappingBuilder () noexcept;

    ~MappingBuilder ();

    void Begin (std::size_t _objectSize) noexcept;

    Mapping End () noexcept;

    FieldId RegisterInt8 (std::size_t _offset) noexcept;

    FieldId RegisterInt16 (std::size_t _offset) noexcept;

    FieldId RegisterInt32 (std::size_t _offset) noexcept;

    FieldId RegisterInt64 (std::size_t _offset) noexcept;

    FieldId RegisterUInt8 (std::size_t _offset) noexcept;

    FieldId RegisterUInt16 (std::size_t _offset) noexcept;

    FieldId RegisterUInt32 (std::size_t _offset) noexcept;

    FieldId RegisterUInt64 (std::size_t _offset) noexcept;

    FieldId RegisterFloat (std::size_t _offset) noexcept;

    FieldId RegisterDouble (std::size_t _offset) noexcept;

    FieldId RegisterString (std::size_t _offset, std::size_t _maxSize) noexcept;

    FieldId RegisterBlock (std::size_t _offset, std::size_t _size) noexcept;

    FieldId RegisterNestedObject (std::size_t _offset, const Mapping &nestedMapping) noexcept;

private:
    void *handle;
};
} // namespace Emergence::StandardLayout
