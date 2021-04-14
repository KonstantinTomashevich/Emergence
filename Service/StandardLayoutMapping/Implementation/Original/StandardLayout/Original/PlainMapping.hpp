#pragma once

#include <cstdlib>
#include <type_traits>

#include <StandardLayout/Field.hpp>
#include <StandardLayout/Mapping.hpp>

namespace Emergence::StandardLayout
{
struct PlainMapping final
{
    static PlainMapping *Allocate (std::size_t _fieldCapacity) noexcept;

    PlainMapping *Reallocate (std::size_t _fieldCapacity) noexcept;

//    const FieldMeta *GetField (FieldId _field) const noexcept;
//
//    FieldMeta *GetField (FieldId _field) noexcept;

    std::size_t references = 0u;
    std::size_t objectSize = 0u;
    std::size_t fieldCount = 0u;
    // inplace_dynamic_array <FieldMeta> fields (fieldCount);
    // unused_memory additionalFieldPlaceholder (0u, infinity);
};

//static_assert (std::is_trivial_v <FieldMeta>);
} // namespace Emergence::StandardLayout