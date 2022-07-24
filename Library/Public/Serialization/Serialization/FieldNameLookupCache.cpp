#include <Serialization/FieldNameLookupCache.hpp>

namespace Emergence::Serialization
{
FieldNameLookupCache::FieldNameLookupCache (const StandardLayout::Mapping &_mapping) noexcept
    : mapping (_mapping),
      cache {Memory::Profiler::AllocationGroup {Memory::UniqueString {"FieldNameLookupCache"}}}
{
}

StandardLayout::Field FieldNameLookupCache::Lookup (Memory::UniqueString _name) noexcept
{
    if (auto iterator = cache.find (_name); iterator != cache.end ())
    {
        return iterator->second;
    }

    for (StandardLayout::Field field : mapping)
    {
        if (field.GetName () == _name)
        {
            cache.emplace (_name, field);
            return field;
        }
    }

    return {};
}

const StandardLayout::Mapping &FieldNameLookupCache::GetTypeMapping () const noexcept
{
    return mapping;
}
} // namespace Emergence::Serialization
