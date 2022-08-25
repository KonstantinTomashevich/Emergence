#pragma once

#include <Asset/Object/TypeManifest.hpp>

#include <Container/Vector.hpp>

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>
#include <StandardLayout/Patch.hpp>

namespace Emergence::Asset::Object
{
struct Declaration final
{
    Memory::UniqueString parent;

    struct Reflection final
    {
        StandardLayout::FieldId parent;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct Body final
{
    Container::Vector<StandardLayout::Patch> fullChangelist {
        Memory::Profiler::AllocationGroup {GetRootAllocationGroup (), Memory::UniqueString {"Body"}}};
};

Body ApplyInheritance (const TypeManifest &_typeManifest,
                       const Body &_parent,
                       const Container::Vector<StandardLayout::Patch> &_childChangelist) noexcept;

void ExtractChildChangelist (const TypeManifest &_typeManifest,
                             const Body &_parent,
                             const Body &_child,
                             Container::Vector<StandardLayout::Patch> &_output) noexcept;
} // namespace Emergence::Asset::Object