#include <cassert>

#include <Galleon/CargoDeck.hpp>

namespace Emergence::Galleon
{
struct TypeMappingPredicate final
{
    bool operator() (const ContainerBase *_container) const noexcept
    {
        return _container->GetTypeMapping () == requiredMapping;
    }

    StandardLayout::Mapping requiredMapping;
};

using namespace Memory::Literals;

CargoDeck::CargoDeck (Memory::UniqueString _name) noexcept
    : name (_name),
      singletonHeap (Memory::Profiler::AllocationGroup {"Singleton"_us}),
      shortTermHeap (Memory::Profiler::AllocationGroup {"ShortTerm"_us}),
      longTermHeap (Memory::Profiler::AllocationGroup {"LongTerm"_us}),
      singletonContainers (singletonHeap),
      shortTermContainers (shortTermHeap),
      longTermContainers (longTermHeap)
{
}

CargoDeck::~CargoDeck () noexcept
{
    // Assert that all containers are detached.
    assert (singletonContainers.empty ());
    assert (shortTermContainers.empty ());
    assert (longTermContainers.empty ());
}

Handling::Handle<SingletonContainer> CargoDeck::AcquireSingletonContainer (const StandardLayout::Mapping &_typeMapping)
{
    auto iterator =
        std::find_if (singletonContainers.begin (), singletonContainers.end (), TypeMappingPredicate {_typeMapping});

    if (iterator != singletonContainers.end ())
    {
        return *iterator;
    }

    auto placeholder = singletonHeap.GetAllocationGroup ().PlaceOnTop ();
    return singletonContainers.emplace_back (new (singletonHeap.Acquire (
        sizeof (SingletonContainer) + _typeMapping.GetObjectSize ())) SingletonContainer (this, _typeMapping));
}

Handling::Handle<ShortTermContainer> CargoDeck::AcquireShortTermContainer (const StandardLayout::Mapping &_typeMapping)
{
    auto iterator =
        std::find_if (shortTermContainers.begin (), shortTermContainers.end (), TypeMappingPredicate {_typeMapping});

    if (iterator != shortTermContainers.end ())
    {
        return *iterator;
    }

    auto placeholder = shortTermHeap.GetAllocationGroup ().PlaceOnTop ();
    return shortTermContainers.emplace_back (new (shortTermHeap.Acquire (sizeof (ShortTermContainer)))
                                                 ShortTermContainer (this, _typeMapping));
}

Handling::Handle<LongTermContainer> CargoDeck::AcquireLongTermContainer (const StandardLayout::Mapping &_typeMapping)
{
    auto iterator =
        std::find_if (longTermContainers.begin (), longTermContainers.end (), TypeMappingPredicate {_typeMapping});

    if (iterator != longTermContainers.end ())
    {
        return *iterator;
    }

    auto placeholder = longTermHeap.GetAllocationGroup ().PlaceOnTop ();
    return longTermContainers.emplace_back (new (longTermHeap.Acquire (sizeof (LongTermContainer)))
                                                LongTermContainer (this, _typeMapping));
}

bool CargoDeck::IsSingletonContainerAllocated (const StandardLayout::Mapping &_typeMapping) const noexcept
{
    return std::find_if (singletonContainers.begin (), singletonContainers.end (),
                         TypeMappingPredicate {_typeMapping}) != singletonContainers.end ();
}

bool CargoDeck::IsShortTermContainerAllocated (const StandardLayout::Mapping &_typeMapping) const noexcept
{
    return std::find_if (shortTermContainers.begin (), shortTermContainers.end (),
                         TypeMappingPredicate {_typeMapping}) != shortTermContainers.end ();
}

bool CargoDeck::IsLongTermContainerAllocated (const StandardLayout::Mapping &_typeMapping) const noexcept
{
    return std::find_if (longTermContainers.begin (), longTermContainers.end (), TypeMappingPredicate {_typeMapping}) !=
           longTermContainers.end ();
}

Memory::UniqueString CargoDeck::GetName () const noexcept
{
    return name;
}

void CargoDeck::DetachContainer (SingletonContainer *_container) noexcept
{
    auto iterator = std::find (singletonContainers.begin (), singletonContainers.end (), _container);
    assert (iterator != singletonContainers.end ());
    singletonContainers.erase (iterator);

    const std::size_t size = sizeof (SingletonContainer) + _container->GetTypeMapping ().GetObjectSize ();
    _container->~SingletonContainer ();
    singletonHeap.Release (_container, size);
}

void CargoDeck::DetachContainer (ShortTermContainer *_container) noexcept
{
    auto iterator = std::find (shortTermContainers.begin (), shortTermContainers.end (), _container);
    assert (iterator != shortTermContainers.end ());
    shortTermContainers.erase (iterator);

    _container->~ShortTermContainer ();
    shortTermHeap.Release (_container, sizeof (ShortTermContainer));
}

void CargoDeck::DetachContainer (LongTermContainer *_container) noexcept
{
    auto iterator = std::find (longTermContainers.begin (), longTermContainers.end (), _container);
    assert (iterator != longTermContainers.end ());
    longTermContainers.erase (iterator);

    _container->~LongTermContainer ();
    longTermHeap.Release (_container, sizeof (LongTermContainer));
}
} // namespace Emergence::Galleon
