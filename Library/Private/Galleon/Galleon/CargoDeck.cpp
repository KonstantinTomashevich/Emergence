#include <Assert/Assert.hpp>

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
      longTerm (Memory::Profiler::AllocationGroup {"LongTerm"_us})
{
}

CargoDeck::~CargoDeck () noexcept
{
    // Assert that all containers are detached.
    EMERGENCE_ASSERT (singleton.IsEmpty ());
    EMERGENCE_ASSERT (shortTerm.IsEmpty ());
    EMERGENCE_ASSERT (longTerm.IsEmpty ());
}

Handling::Handle<SingletonContainer> CargoDeck::AcquireSingletonContainer (const StandardLayout::Mapping &_typeMapping)
{
    auto iterator = std::find_if (singleton.Begin (), singleton.End (), TypeMappingPredicate {_typeMapping});
    if (iterator != singleton.End ())
    {
        return &*iterator;
    }

    auto placeholder = singleton.GetAllocationGroup ().PlaceOnTop ();
    return &singleton.Acquire (this, _typeMapping);
}

Handling::Handle<ShortTermContainer> CargoDeck::AcquireShortTermContainer (const StandardLayout::Mapping &_typeMapping)
{
    auto iterator = std::find_if (shortTerm.Begin (), shortTerm.End (), TypeMappingPredicate {_typeMapping});
    if (iterator != shortTerm.End ())
    {
        return &*iterator;
    }

    auto placeholder = shortTerm.GetAllocationGroup ().PlaceOnTop ();
    return &shortTerm.Acquire (this, _typeMapping);
}

Handling::Handle<LongTermContainer> CargoDeck::AcquireLongTermContainer (const StandardLayout::Mapping &_typeMapping)
{
    auto iterator = std::find_if (longTerm.Begin (), longTerm.End (), TypeMappingPredicate {_typeMapping});
    if (iterator != longTerm.End ())
    {
        return &*iterator;
    }

    auto placeholder = longTerm.GetAllocationGroup ().PlaceOnTop ();
    return &longTerm.Acquire (this, _typeMapping);
}

bool CargoDeck::IsSingletonContainerAllocated (const StandardLayout::Mapping &_typeMapping) const noexcept
{
    return std::find_if (singleton.Begin (), singleton.End (), TypeMappingPredicate {_typeMapping}) != singleton.End ();
}

bool CargoDeck::IsShortTermContainerAllocated (const StandardLayout::Mapping &_typeMapping) const noexcept
{
    return std::find_if (shortTerm.Begin (), shortTerm.End (), TypeMappingPredicate {_typeMapping}) != shortTerm.End ();
}

bool CargoDeck::IsLongTermContainerAllocated (const StandardLayout::Mapping &_typeMapping) const noexcept
{
    return std::find_if (longTerm.Begin (), longTerm.End (), TypeMappingPredicate {_typeMapping}) != longTerm.End ();
}

Memory::UniqueString CargoDeck::GetName () const noexcept
{
    return name;
}

void CargoDeck::DetachContainer (SingletonContainer *_container) noexcept
{
    singleton.Release (*_container);
}

void CargoDeck::DetachContainer (ShortTermContainer *_container) noexcept
{
    shortTerm.Release (*_container);
}

void CargoDeck::DetachContainer (LongTermContainer *_container) noexcept
{
    longTerm.Release (*_container);
}
} // namespace Emergence::Galleon
