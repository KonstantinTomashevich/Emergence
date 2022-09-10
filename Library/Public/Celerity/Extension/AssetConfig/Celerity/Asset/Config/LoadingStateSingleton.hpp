#pragma once

#include <Container/String.hpp>
#include <Container/Vector.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct AssetConfigLoadingStateSingleton final
{
    struct TypeState final
    {
        StandardLayout::Mapping type;
        StandardLayout::FieldId nameField;
        Container::String folder;
        bool loaded = false;
    };

    bool pathMappingLoaded = false;
    Container::Vector<TypeState> typeStates {Memory::Profiler::AllocationGroup::Top ()};

    struct Reflection final
    {
        StandardLayout::FieldId pathMappingLoaded;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
