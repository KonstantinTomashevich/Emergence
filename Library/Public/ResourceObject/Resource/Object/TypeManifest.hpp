#pragma once

#include <Container/HashMap.hpp>

#include <Resource/Object/AllocationGroup.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Resource::Object
{
/// \brief This type should be used to uniquely identify object parts for inheritance algorithms.
using UniqueId = std::uint64_t;

/// \brief Contains metadata attached to register object part type.
struct TypeInfo final
{
    /// \brief Field that contains unique id value for object part of this type.
    /// \details Id must be unique among all parts of the same type, but parts of different types may have equal ids.
    StandardLayout::FieldId uniqueId;
};

/// \brief Contains metadata for all types, instances of which can be used as object parts.
class TypeManifest final
{
public:
    /// \brief Registers metadata for given type.
    void Register (const StandardLayout::Mapping &_mapping, const TypeInfo &_info) noexcept;

    /// \return Metadata for given type or `nullptr` if it's not registered.
    [[nodiscard]] const TypeInfo *Get (const StandardLayout::Mapping &_mapping) const noexcept;

    /// \return Map of all registered types with their metadata.
    [[nodiscard]] const Container::HashMap<StandardLayout::Mapping, TypeInfo> &GetMap () const noexcept;

private:
    Container::HashMap<StandardLayout::Mapping, TypeInfo> infos {
        Memory::Profiler::AllocationGroup {GetRootAllocationGroup (), Memory::UniqueString {"TypeManifest"}}};
};
} // namespace Emergence::Resource::Object
