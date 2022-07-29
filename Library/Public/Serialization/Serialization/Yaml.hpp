#pragma once

#include <istream>
#include <ostream>

#include <Container/HashMap.hpp>
#include <Container/Vector.hpp>

#include <Serialization/FieldNameLookupCache.hpp>

#include <StandardLayout/Mapping.hpp>
#include <StandardLayout/Patch.hpp>
#include <StandardLayout/PatchBuilder.hpp>

namespace Emergence::Serialization::Yaml
{
void SerializeObject (std::ostream &_output, const void *_object, const StandardLayout::Mapping &_mapping) noexcept;

bool DeserializeObject (std::istream &_input, void *_object, FieldNameLookupCache &_cache) noexcept;

void SerializePatch (std::ostream &_output, const StandardLayout::Patch &_patch) noexcept;

bool DeserializePatch (std::istream &_input,
                       StandardLayout::PatchBuilder &_builder,
                       FieldNameLookupCache &_cache) noexcept;

bool DeserializePatchBundle (std::istream &_input,
                             Container::Vector<StandardLayout::Patch> &_output,
                             class BundleDeserializationContext &_context) noexcept;

class BundleDeserializationContext final
{
public:
    void RegisterType (const StandardLayout::Mapping &_mapping) noexcept;

    FieldNameLookupCache *RequestCache (Memory::UniqueString _typeName) noexcept;

private:
    Container::HashMap<Memory::UniqueString, FieldNameLookupCache> cachesByTypeName {
        Memory::Profiler::AllocationGroup {Memory::UniqueString {"BundleDeserializationContext"}}};
};
} // namespace Emergence::Serialization::Yaml
