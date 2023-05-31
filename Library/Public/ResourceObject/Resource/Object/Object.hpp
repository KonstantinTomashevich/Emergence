#pragma once

#include <Container/Vector.hpp>

#include <Memory/UniqueString.hpp>

#include <Resource/Object/TypeManifest.hpp>

#include <StandardLayout/Mapping.hpp>
#include <StandardLayout/Patch.hpp>

namespace Emergence::Resource::Object
{
/// \brief Contains information about one particular part of resource object.
struct ObjectComponent final
{
    /// \brief Patch that transform this part into required state.
    StandardLayout::Patch component;

    struct Reflection final
    {
        StandardLayout::FieldId component;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

/// \brief Contains full serializable information about resource object.
/// \details Mapping is called ResourceObject, because serialization and mappings know nothing about namespaces.
struct Object final
{
    /// \brief Name of the objects that is logical parent for this object or empty value if no parent is required.
    /// \details Parent content will be merged with this object content to form final ready-to-use object.
    Memory::UniqueString parent;

    /// \brief Changelist will all object components.
    /// \details Depending on object loading stage, might be local-only or already merged with parent changelist.
    ///          If object is fully loaded, then changelists are merged.
    Container::Vector<ObjectComponent> changelist {
        Memory::Profiler::AllocationGroup {GetRootAllocationGroup (), Memory::UniqueString {"Object"}}};

    struct Reflection final
    {
        StandardLayout::FieldId parent;
        StandardLayout::FieldId changelist;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

/// \brief Merges parent object changelist into given child object.
void ApplyInheritance (const TypeManifest &_typeManifest, const Object &_parent, Object &_child) noexcept;

/// \brief Extracts child local changelist by comparing child object with parent object.
void ExtractChildChangelist (const TypeManifest &_typeManifest,
                             const Object &_parent,
                             const Object &_child,
                             Container::Vector<ObjectComponent> &_output) noexcept;
} // namespace Emergence::Resource::Object
