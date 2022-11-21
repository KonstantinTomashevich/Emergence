#pragma once

#include <Container/Vector.hpp>

#include <Memory/UniqueString.hpp>

#include <Resource/Object/TypeManifest.hpp>

#include <StandardLayout/Mapping.hpp>
#include <StandardLayout/Patch.hpp>

namespace Emergence::Resource::Object
{
/// \brief Declaration contains various information about an object, but has no information about object content.
struct Declaration final
{
    /// \brief Name of the objects that is logical parent for this object or empty value if no parent is required.
    /// \details Parent content will be merged with this object content to form final ready-to-use object.
    Memory::UniqueString parent;

    struct Reflection final
    {
        StandardLayout::FieldId parent;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

/// \brief Body contains information about object content in the form of changelist.
struct Body final
{
    /// \brief Ready-to-use changelist that is already merged with Declaration::parent changelist (if parent exists).
    Container::Vector<StandardLayout::Patch> fullChangelist {
        Memory::Profiler::AllocationGroup {GetRootAllocationGroup (), Memory::UniqueString {"Body"}}};
};

/// \brief Forms ready-to-use body by merging parent body with child changelist.
Body ApplyInheritance (const TypeManifest &_typeManifest,
                       const Body &_parent,
                       const Container::Vector<StandardLayout::Patch> &_childChangelist) noexcept;

/// \brief Extracts child changelist by comparing child body with parent body.
void ExtractChildChangelist (const TypeManifest &_typeManifest,
                             const Body &_parent,
                             const Body &_child,
                             Container::Vector<StandardLayout::Patch> &_output) noexcept;
} // namespace Emergence::Resource::Object
