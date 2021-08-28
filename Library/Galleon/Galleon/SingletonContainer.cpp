#include <cassert>
#include <cstdlib>

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

SingletonContainer::SingletonContainer (CargoDeck *_deck, StandardLayout::Mapping _typeMapping) noexcept
    : ContainerBase (_deck, std::move (_typeMapping))
{
    // Fill singleton storage with zeros even in release builds
    // to make uninitialized-access checking and debugging easier.
    memset (&storage, 0u, typeMapping.GetObjectSize ());
}

SingletonContainer::~SingletonContainer () noexcept
{
    assert (deck);
    deck->DetachContainer (this);
}

void *SingletonContainer::operator new (std::size_t, const StandardLayout::Mapping &_typeMapping) noexcept
{
    return malloc (sizeof (SingletonContainer) + _typeMapping.GetObjectSize ());
}

void SingletonContainer::operator delete (void *_pointer) noexcept
{
    free (_pointer);
}
} // namespace Emergence::Galleon