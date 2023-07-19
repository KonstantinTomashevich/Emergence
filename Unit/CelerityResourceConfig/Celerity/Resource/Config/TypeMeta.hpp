#pragma once

#include <CelerityResourceConfigApi.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Contains meta information about config type that is needed for resource config processing.
struct CelerityResourceConfigApi ResourceConfigTypeMeta final
{
    /// \brief Type of a config.
    StandardLayout::Mapping mapping;

    /// \brief Field that contains config name.
    StandardLayout::FieldId nameField;
};
} // namespace Emergence::Celerity

EMERGENCE_MEMORY_DEFAULT_ALLOCATION_GROUP (CelerityResourceConfigApi, Celerity::ResourceConfigTypeMeta)
