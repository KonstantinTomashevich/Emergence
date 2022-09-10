#pragma once

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct AssetConfigTypeMeta final
{
    StandardLayout::Mapping mapping;
    StandardLayout::FieldId nameField;
};
} // namespace Emergence::Celerity

EMERGENCE_MEMORY_DEFAULT_ALLOCATION_GROUP (Celerity::AssetConfigTypeMeta)
