#include <cassert>
#include <cstdlib>
#include <cstring>

#include <Galleon/CargoDeck.hpp>
#include <Galleon/SingletonContainer.hpp>

namespace Emergence::Galleon
{
SingletonContainer::FetchQuery::Cursor::Cursor (const SingletonContainer::FetchQuery::Cursor &_other) noexcept
    : container (_other.container)
{
    assert (container);
    container->accessCounter.RegisterReadAccess ();
}

SingletonContainer::FetchQuery::Cursor::~Cursor () noexcept
{
    if (container)
    {
        container->accessCounter.UnregisterReadAccess ();
    }
}

const void *SingletonContainer::FetchQuery::Cursor::operator* () const noexcept
{
    assert (container);
    return &container->storage;
}

SingletonContainer::FetchQuery::Cursor::Cursor (Handling::Handle<SingletonContainer> _container) noexcept
    : container (std::move (_container))
{
    assert (container);
    container->accessCounter.RegisterReadAccess ();
}

SingletonContainer::FetchQuery::Cursor SingletonContainer::FetchQuery::Execute () const noexcept
{
    return Cursor (container);
}

Handling::Handle<SingletonContainer> SingletonContainer::FetchQuery::GetContainer () const noexcept
{
    assert (container);
    return container;
}

SingletonContainer::FetchQuery::FetchQuery (Handling::Handle<SingletonContainer> _container) noexcept
    : container (std::move (_container))
{
    assert (container);
}

SingletonContainer::ModifyQuery::Cursor::~Cursor () noexcept
{
    if (container)
    {
        container->accessCounter.UnregisterWriteAccess ();
    }
}

void *SingletonContainer::ModifyQuery::Cursor::operator* () const noexcept
{
    assert (container);
    return &container->storage;
}

SingletonContainer::ModifyQuery::Cursor::Cursor (Handling::Handle<SingletonContainer> _container) noexcept
    : container (std::move (_container))
{
    assert (container);
    container->accessCounter.RegisterWriteAccess ();
}

SingletonContainer::ModifyQuery::Cursor SingletonContainer::ModifyQuery::Execute () const noexcept
{
    return Cursor (container);
}

Handling::Handle<SingletonContainer> SingletonContainer::ModifyQuery::GetContainer () const noexcept
{
    assert (container);
    return container;
}

SingletonContainer::ModifyQuery::ModifyQuery (Handling::Handle<SingletonContainer> _container) noexcept
    : container (std::move (_container))
{
    assert (container);
}

SingletonContainer::FetchQuery SingletonContainer::Fetch () noexcept
{
    return FetchQuery (this);
}

SingletonContainer::ModifyQuery SingletonContainer::Modify () noexcept
{
    return ModifyQuery (this);
}

void SingletonContainer::LastReferenceUnregistered () noexcept
{
    assert (deck);
    deck->DetachContainer (this);
}

void SingletonContainer::SetUnsafeFetchAllowed (bool _allowed) noexcept
{
    accessCounter.SetUnsafeFetchAllowed (_allowed);
}

SingletonContainer::SingletonContainer (CargoDeck *_deck, StandardLayout::Mapping _typeMapping) noexcept
    : ContainerBase (_deck, std::move (_typeMapping)),
      usedAllocationGroup (typeMapping.GetName ())
{
    auto placeholder = usedAllocationGroup.PlaceOnTop ();
    typeMapping.Construct (&storage);

    // Move singleton memory usage from parent group into our internal group.
    usedAllocationGroup.Parent ().Release (typeMapping.GetObjectSize ());
    usedAllocationGroup.Parent ().Free (typeMapping.GetObjectSize ());
    usedAllocationGroup.Allocate (typeMapping.GetObjectSize ());
    usedAllocationGroup.Acquire (typeMapping.GetObjectSize ());
}

SingletonContainer::~SingletonContainer () noexcept
{
    typeMapping.Destruct (&storage);

    // Move singleton memory usage from internal group into parent group, because it was actually acquired
    // from parent allocator. Otherwise, profiler will detect deallocation of unregistered memory and crash.
    usedAllocationGroup.Release (typeMapping.GetObjectSize ());
    usedAllocationGroup.Free (typeMapping.GetObjectSize ());
    usedAllocationGroup.Parent ().Allocate (typeMapping.GetObjectSize ());
    usedAllocationGroup.Parent ().Acquire (typeMapping.GetObjectSize ());
}
} // namespace Emergence::Galleon
