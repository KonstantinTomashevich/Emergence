#pragma once

#include <Asset/Object/AllocationGroup.hpp>

#include <Container/HashMap.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Asset::Object
{
using UniqueId = std::uint64_t;

struct TypeInfo final
{
    StandardLayout::FieldId uniqueId;
};

class TypeManifest final
{
public:
    void Register (const StandardLayout::Mapping &_mapping, const TypeInfo &_info) noexcept;

    [[nodiscard]] const TypeInfo *Get (const StandardLayout::Mapping &_mapping) const noexcept;

    [[nodiscard]] const Container::HashMap<StandardLayout::Mapping, TypeInfo> &GetMap () const noexcept;

private:
    Container::HashMap<StandardLayout::Mapping, TypeInfo> infos {
        Memory::Profiler::AllocationGroup {GetRootAllocationGroup (), Memory::UniqueString {"TypeManifest"}}};
};
} // namespace Emergence::Asset::Object
