#pragma once

#include <Container/HashSet.hpp>
#include <Container/TypedOrderedPool.hpp>

#include <Galleon/LongTermContainer.hpp>
#include <Galleon/ShortTermContainer.hpp>
#include <Galleon/SingletonContainer.hpp>

#include <Handling/Handle.hpp>

#include <Memory/Heap.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Galleon
{
class CargoDeck final
{
public:
    CargoDeck (Memory::UniqueString _name) noexcept;

    /// CargoDeck manages lots of storages with lots of objects, therefore it's not optimal to copy it.
    CargoDeck (const CargoDeck &_other) = delete;

    /// Move constructor could be useful, but we don't implement it
    /// because it's useless for Warehouse implementation.
    CargoDeck (CargoDeck &&_other) = delete;

    /// \invariant There is no handles to any container and no prepared queries, that belong this deck.
    ~CargoDeck () noexcept;

    Handling::Handle<SingletonContainer> AcquireSingletonContainer (const StandardLayout::Mapping &_typeMapping);

    Handling::Handle<ShortTermContainer> AcquireShortTermContainer (const StandardLayout::Mapping &_typeMapping);

    Handling::Handle<LongTermContainer> AcquireLongTermContainer (const StandardLayout::Mapping &_typeMapping);

    [[nodiscard]] bool IsSingletonContainerAllocated (const StandardLayout::Mapping &_typeMapping) const noexcept;

    [[nodiscard]] bool IsShortTermContainerAllocated (const StandardLayout::Mapping &_typeMapping) const noexcept;

    [[nodiscard]] bool IsLongTermContainerAllocated (const StandardLayout::Mapping &_typeMapping) const noexcept;

    void SetGarbageCollectionEnabled (const StandardLayout::Mapping &_typeMapping, bool _enabled) noexcept;

    [[nodiscard]] Memory::UniqueString GetName () const noexcept;

    /// CargoDeck manages lots of storages with lots of objects, therefore it's not optimal to copy assign it.
    CargoDeck &operator= (const CargoDeck &_other) = delete;

    /// Move assign could be useful, but we don't implement it
    /// because it's useless for Warehouse implementation.
    CargoDeck &operator= (CargoDeck &&_other) = delete;

private:
    friend class SingletonContainer;

    friend class ShortTermContainer;

    friend class LongTermContainer;

    /// VisualizationDriver for Warehouse service should be able to directly access containers.
    friend class VisualizationDriver;

    void DetachContainer (SingletonContainer *_container) noexcept;

    void DetachContainer (ShortTermContainer *_container) noexcept;

    void DetachContainer (LongTermContainer *_container) noexcept;

    Memory::UniqueString name;

    Container::TypedOrderedPool<SingletonContainer> singleton;
    Container::TypedOrderedPool<ShortTermContainer> shortTerm;
    Container::TypedOrderedPool<LongTermContainer> longTerm;
    Container::HashSet<StandardLayout::Mapping> garbageCollectionDisabled;
};
} // namespace Emergence::Galleon
