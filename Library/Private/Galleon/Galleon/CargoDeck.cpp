#include <cassert>

#include <Galleon/CargoDeck.hpp>

#include <Memory/Profiler/AllocationGroup.hpp>

namespace Emergence::Galleon
{
static const Memory::UniqueString CONTAINER_ARRAYS {"ContainerArrays"};

namespace MP = Memory::Profiler;

struct TypeMappingPredicate final
{
    bool operator() (const ContainerBase *_container) const noexcept
    {
        return _container->GetTypeMapping () == requiredMapping;
    }

    StandardLayout::Mapping requiredMapping;
};

CargoDeck::CargoDeck (Memory::UniqueString _name) noexcept
    : name (_name),
      containers {.singleton = MP::ConstructWithinGroup<decltype (containers.singleton)> (_name, CONTAINER_ARRAYS),
                  .shortTerm = MP::ConstructWithinGroup<decltype (containers.shortTerm)> (_name, CONTAINER_ARRAYS),
                  .longTerm = MP::ConstructWithinGroup<decltype (containers.longTerm)> (_name, CONTAINER_ARRAYS)}
{
}

CargoDeck::~CargoDeck () noexcept
{
    // Assert that all containers are detached.
    assert (containers.singleton.empty ());
    assert (containers.shortTerm.empty ());
    assert (containers.longTerm.empty ());
}

Handling::Handle<SingletonContainer> CargoDeck::AcquireSingletonContainer (const StandardLayout::Mapping &_typeMapping)
{
    auto iterator =
        std::find_if (containers.singleton.begin (), containers.singleton.end (), TypeMappingPredicate {_typeMapping});

    if (iterator != containers.singleton.end ())
    {
        return *iterator;
    }

    return containers.singleton.emplace_back (new (_typeMapping) SingletonContainer (this, _typeMapping));
}

Handling::Handle<ShortTermContainer> CargoDeck::AcquireShortTermContainer (const StandardLayout::Mapping &_typeMapping)
{
    auto iterator =
        std::find_if (containers.shortTerm.begin (), containers.shortTerm.end (), TypeMappingPredicate {_typeMapping});

    if (iterator != containers.shortTerm.end ())
    {
        return *iterator;
    }

    return containers.shortTerm.emplace_back (new ShortTermContainer (this, _typeMapping));
}

Handling::Handle<LongTermContainer> CargoDeck::AcquireLongTermContainer (const StandardLayout::Mapping &_typeMapping)
{
    auto iterator =
        std::find_if (containers.longTerm.begin (), containers.longTerm.end (), TypeMappingPredicate {_typeMapping});

    if (iterator != containers.longTerm.end ())
    {
        return *iterator;
    }

    return containers.longTerm.emplace_back (new LongTermContainer (this, _typeMapping));
}

bool CargoDeck::IsSingletonContainerAllocated (const StandardLayout::Mapping &_typeMapping) const noexcept
{
    return std::find_if (containers.singleton.begin (), containers.singleton.end (),
                         TypeMappingPredicate {_typeMapping}) != containers.singleton.end ();
}

bool CargoDeck::IsShortTermContainerAllocated (const StandardLayout::Mapping &_typeMapping) const noexcept
{
    return std::find_if (containers.shortTerm.begin (), containers.shortTerm.end (),
                         TypeMappingPredicate {_typeMapping}) != containers.shortTerm.end ();
}

bool CargoDeck::IsLongTermContainerAllocated (const StandardLayout::Mapping &_typeMapping) const noexcept
{
    return std::find_if (containers.longTerm.begin (), containers.longTerm.end (),
                         TypeMappingPredicate {_typeMapping}) != containers.longTerm.end ();
}

Memory::UniqueString CargoDeck::GetName () const noexcept
{
    return name;
}

void CargoDeck::DetachContainer (SingletonContainer *_container) noexcept
{
    auto iterator = std::find (containers.singleton.begin (), containers.singleton.end (), _container);
    assert (iterator != containers.singleton.end ());
    containers.singleton.erase (iterator);
}

void CargoDeck::DetachContainer (ShortTermContainer *_container) noexcept
{
    auto iterator = std::find (containers.shortTerm.begin (), containers.shortTerm.end (), _container);
    assert (iterator != containers.shortTerm.end ());
    containers.shortTerm.erase (iterator);
}

void CargoDeck::DetachContainer (LongTermContainer *_container) noexcept
{
    auto iterator = std::find (containers.longTerm.begin (), containers.longTerm.end (), _container);
    assert (iterator != containers.longTerm.end ());
    containers.longTerm.erase (iterator);
}
} // namespace Emergence::Galleon
