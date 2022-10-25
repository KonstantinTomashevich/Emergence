#include <Galleon/CargoDeck.hpp>
#include <Galleon/SingletonContainer.hpp>

namespace Emergence::Galleon
{
SingletonContainer::FetchQuery::Cursor::Cursor (const SingletonContainer::FetchQuery::Cursor &_other) noexcept
    : container (_other.container)
{
    EMERGENCE_ASSERT (container);
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
    EMERGENCE_ASSERT (container);
    return container->singletonInstance;
}

SingletonContainer::FetchQuery::Cursor::Cursor (Handling::Handle<SingletonContainer> _container) noexcept
    : container (std::move (_container))
{
    EMERGENCE_ASSERT (container);
    container->accessCounter.RegisterReadAccess ();
}

SingletonContainer::FetchQuery::Cursor SingletonContainer::FetchQuery::Execute () const noexcept
{
    return Cursor (container);
}

Handling::Handle<SingletonContainer> SingletonContainer::FetchQuery::GetContainer () const noexcept
{
    EMERGENCE_ASSERT (container);
    return container;
}

SingletonContainer::FetchQuery::FetchQuery (Handling::Handle<SingletonContainer> _container) noexcept
    : container (std::move (_container))
{
    EMERGENCE_ASSERT (container);
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
    EMERGENCE_ASSERT (container);
    return container->singletonInstance;
}

SingletonContainer::ModifyQuery::Cursor::Cursor (Handling::Handle<SingletonContainer> _container) noexcept
    : container (std::move (_container))
{
    EMERGENCE_ASSERT (container);
    container->accessCounter.RegisterWriteAccess ();
}

SingletonContainer::ModifyQuery::Cursor SingletonContainer::ModifyQuery::Execute () const noexcept
{
    return Cursor (container);
}

Handling::Handle<SingletonContainer> SingletonContainer::ModifyQuery::GetContainer () const noexcept
{
    EMERGENCE_ASSERT (container);
    return container;
}

SingletonContainer::ModifyQuery::ModifyQuery (Handling::Handle<SingletonContainer> _container) noexcept
    : container (std::move (_container))
{
    EMERGENCE_ASSERT (container);
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
    EMERGENCE_ASSERT (deck);
    deck->DetachContainer (this);
}

void SingletonContainer::SetUnsafeFetchAllowed (bool _allowed) noexcept
{
    accessCounter.SetUnsafeFetchAllowed (_allowed);
}

SingletonContainer::SingletonContainer (CargoDeck *_deck, StandardLayout::Mapping _typeMapping) noexcept
    : ContainerBase (_deck, std::move (_typeMapping)),
      singletonHeap (Memory::Profiler::AllocationGroup (typeMapping.GetName ()))
{
    singletonInstance = singletonHeap.Acquire (typeMapping.GetObjectSize (), typeMapping.GetObjectAlignment ());
    typeMapping.Construct (singletonInstance);
}

SingletonContainer::~SingletonContainer () noexcept
{
    typeMapping.Destruct (singletonInstance);
    singletonHeap.Release (singletonInstance, typeMapping.GetObjectSize ());
}
} // namespace Emergence::Galleon
