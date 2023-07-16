#include <Assert/Assert.hpp>

#include <Container/Algorithm.hpp>

#include <Galleon/CargoDeck.hpp>

namespace Emergence::Galleon
{
struct TypeMappingPredicate final
{
    bool operator() (const ContainerBase &_container) const noexcept
    {
        return _container.GetTypeMapping () == requiredMapping;
    }

    StandardLayout::Mapping requiredMapping;
};

using namespace Memory::Literals;

CargoDeck::CargoDeck (Memory::UniqueString _name) noexcept
    : name (_name),
      singleton (Memory::Profiler::AllocationGroup {"Singleton"_us}),
      shortTerm (Memory::Profiler::AllocationGroup {"ShortTerm"_us}),
      longTerm (Memory::Profiler::AllocationGroup {"LongTerm"_us}),
      garbageCollectionDisabled (Memory::Profiler::AllocationGroup {"GarbageCollectionDisabledSet"_us})
{
}

// NOLINTNEXTLINE(modernize-use-equals-default): It's not actually default unless assertions are disabled.
CargoDeck::~CargoDeck () noexcept
{
    // Assert that all containers are either detached or exist only due to garbage collection disabled flag.
    // We do not need to detach them as they'll be automatically cleared by typed pool destructors.

#if defined(EMERGENCE_ASSERT_ENABLED)
    for (SingletonContainer &container : singleton)
    {
        EMERGENCE_ASSERT (container.GetReferenceCount () == 0u);
        EMERGENCE_ASSERT (garbageCollectionDisabled.contains (container.GetTypeMapping ()));
    }

    for (ShortTermContainer &container : shortTerm)
    {
        EMERGENCE_ASSERT (container.GetReferenceCount () == 0u);
        EMERGENCE_ASSERT (garbageCollectionDisabled.contains (container.GetTypeMapping ()));
    }

    for (LongTermContainer &container : longTerm)
    {
        EMERGENCE_ASSERT (container.GetReferenceCount () == 0u);
        EMERGENCE_ASSERT (garbageCollectionDisabled.contains (container.GetTypeMapping ()));
    }
#endif
}

Handling::Handle<SingletonContainer> CargoDeck::AcquireSingletonContainer (const StandardLayout::Mapping &_typeMapping)
{
    auto iterator = Container::FindIf (singleton.Begin (), singleton.End (), TypeMappingPredicate {_typeMapping});
    if (iterator != singleton.End ())
    {
        return &*iterator;
    }

    auto placeholder = singleton.GetAllocationGroup ().PlaceOnTop ();
    return &singleton.Acquire (this, _typeMapping);
}

Handling::Handle<ShortTermContainer> CargoDeck::AcquireShortTermContainer (const StandardLayout::Mapping &_typeMapping)
{
    auto iterator = Container::FindIf (shortTerm.Begin (), shortTerm.End (), TypeMappingPredicate {_typeMapping});
    if (iterator != shortTerm.End ())
    {
        return &*iterator;
    }

    auto placeholder = shortTerm.GetAllocationGroup ().PlaceOnTop ();
    return &shortTerm.Acquire (this, _typeMapping);
}

Handling::Handle<LongTermContainer> CargoDeck::AcquireLongTermContainer (const StandardLayout::Mapping &_typeMapping)
{
    auto iterator = Container::FindIf (longTerm.Begin (), longTerm.End (), TypeMappingPredicate {_typeMapping});
    if (iterator != longTerm.End ())
    {
        return &*iterator;
    }

    auto placeholder = longTerm.GetAllocationGroup ().PlaceOnTop ();
    return &longTerm.Acquire (this, _typeMapping);
}

bool CargoDeck::IsSingletonContainerAllocated (const StandardLayout::Mapping &_typeMapping) const noexcept
{
    return Container::FindIf (singleton.Begin (), singleton.End (), TypeMappingPredicate {_typeMapping}) !=
           singleton.End ();
}

bool CargoDeck::IsShortTermContainerAllocated (const StandardLayout::Mapping &_typeMapping) const noexcept
{
    return Container::FindIf (shortTerm.Begin (), shortTerm.End (), TypeMappingPredicate {_typeMapping}) !=
           shortTerm.End ();
}

bool CargoDeck::IsLongTermContainerAllocated (const StandardLayout::Mapping &_typeMapping) const noexcept
{
    return Container::FindIf (longTerm.Begin (), longTerm.End (), TypeMappingPredicate {_typeMapping}) !=
           longTerm.End ();
}

void CargoDeck::SetGarbageCollectionEnabled (const StandardLayout::Mapping &_typeMapping, bool _enabled) noexcept
{
    if (_enabled)
    {
        garbageCollectionDisabled.erase (_typeMapping);
        auto singletonIterator =
            Container::FindIf (singleton.Begin (), singleton.End (), TypeMappingPredicate {_typeMapping});

        if (singletonIterator != singleton.End () && (*singletonIterator).GetReferenceCount () == 0u)
        {
            DetachContainer (&*singletonIterator);
        }

        auto shortTermIterator =
            Container::FindIf (shortTerm.Begin (), shortTerm.End (), TypeMappingPredicate {_typeMapping});

        if (shortTermIterator != shortTerm.End () && (*shortTermIterator).GetReferenceCount () == 0u)
        {
            DetachContainer (&*shortTermIterator);
        }

        auto longTermIterator =
            Container::FindIf (longTerm.Begin (), longTerm.End (), TypeMappingPredicate {_typeMapping});
        if (longTermIterator != longTerm.End () && (*longTermIterator).GetReferenceCount () == 0u)
        {
            DetachContainer (&*longTermIterator);
        }
    }
    else
    {
        garbageCollectionDisabled.emplace (_typeMapping);
    }
}

Memory::UniqueString CargoDeck::GetName () const noexcept
{
    return name;
}

void CargoDeck::DetachContainer (SingletonContainer *_container) noexcept
{
    if (!garbageCollectionDisabled.contains (_container->GetTypeMapping ()))
    {
        singleton.Release (*_container);
    }
}

void CargoDeck::DetachContainer (ShortTermContainer *_container) noexcept
{
    if (!garbageCollectionDisabled.contains (_container->GetTypeMapping ()))
    {
        shortTerm.Release (*_container);
    }
}

void CargoDeck::DetachContainer (LongTermContainer *_container) noexcept
{
    if (!garbageCollectionDisabled.contains (_container->GetTypeMapping ()))
    {
        longTerm.Release (*_container);
    }
}
} // namespace Emergence::Galleon
