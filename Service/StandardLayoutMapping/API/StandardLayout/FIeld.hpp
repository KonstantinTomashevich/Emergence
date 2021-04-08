#pragma once

#include <cstdint>

namespace Emergence::StandardLayout
{
using FieldId = uint_fast64_t;

/// \brief Defines mapping-independent field space projection rule.
///
/// \param objectField id of any field in class `Y`, that contains structure of class `X`.
/// \param nestedField id of any field from class `X`.
/// \return unique id (among `Y` class fields) for `nestedField` of `X` from `objectField` of `Y`.
FieldId ProjectNestedField (FieldId objectField, FieldId nestedField) noexcept;

enum class FieldComparisonType
{
    INT,
    UINT,
    FLOAT,
    STRING,
    BLOCK
};

struct FieldMeta final
{
public:
    std::size_t offset = 0u;
    std::size_t size = 0u;
    FieldComparisonType comparisonType = FieldComparisonType::BLOCK;

    void *GetValue (void *_object) const noexcept;
};
} // namespace Emergence::StandardLayout
