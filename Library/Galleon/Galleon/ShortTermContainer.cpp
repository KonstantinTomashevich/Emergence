#include <cassert>

#include <Galleon/CargoDeck.hpp>
#include <Galleon/ShortTermContainer.hpp>

namespace Emergence::Galleon
{
ShortTermContainer::InsertQuery::Cursor::~Cursor () noexcept
{
    if (container)
    {
        container->accessCounter.UnregisterWriteAccess ();
    }
}

void *ShortTermContainer::InsertQuery::Cursor::operator ++ () noexcept
{
    assert (container);
    return container->objects.emplace_back (container->pool.Acquire ());
}

ShortTermContainer::InsertQuery::Cursor::Cursor (Handling::Handle <ShortTermContainer> _container) noexcept
    : container (std::move (_container))
{
    assert (container);
    container->accessCounter.RegisterWriteAccess ();
}

ShortTermContainer::InsertQuery::Cursor ShortTermContainer::InsertQuery::Execute () const noexcept
{
    return Cursor (container);
}

ShortTermContainer::InsertQuery::InsertQuery (Handling::Handle <ShortTermContainer> _container) noexcept
    : container (std::move (_container))
{
    assert (container);
}

ShortTermContainer::FetchQuery::Cursor::Cursor (
    const ShortTermContainer::FetchQuery::Cursor &_other) noexcept = default;

ShortTermContainer::FetchQuery::Cursor::Cursor (ShortTermContainer::FetchQuery::Cursor &&_other) noexcept = default;

ShortTermContainer::FetchQuery::Cursor::~Cursor () noexcept
{
    if (container)
    {
        container->accessCounter.UnregisterReadAccess ();
    }
}

const void *ShortTermContainer::FetchQuery::Cursor::operator * () const noexcept
{
    assert (container);
    return iterator != end ? *iterator : nullptr;
}

ShortTermContainer::FetchQuery::Cursor &ShortTermContainer::FetchQuery::Cursor::operator ++ () noexcept
{
    assert (container);
    assert (iterator != end);
    ++iterator;
    return *this;
}

ShortTermContainer::FetchQuery::Cursor::Cursor (Handling::Handle <ShortTermContainer> _container) noexcept
    : container (std::move (_container)),
      iterator (container->objects.cbegin ()),
      end (container->objects.cend ())
{
    assert (container);
    container->accessCounter.RegisterReadAccess ();
}

ShortTermContainer::FetchQuery::Cursor ShortTermContainer::FetchQuery::Execute () const noexcept
{
    return Cursor (container);
}

ShortTermContainer::FetchQuery::FetchQuery (Handling::Handle <ShortTermContainer> _container) noexcept
    : container ((std::move (_container)))
{
    assert (container);
}

ShortTermContainer::ModifyQuery::Cursor::Cursor (ShortTermContainer::ModifyQuery::Cursor &&_other) noexcept = default;

ShortTermContainer::ModifyQuery::Cursor::~Cursor () noexcept
{
    if (container)
    {
        container->accessCounter.UnregisterWriteAccess ();
    }
}

void *ShortTermContainer::ModifyQuery::Cursor::operator * () noexcept
{
    assert (container);
    return iterator != end ? *iterator : nullptr;
}

ShortTermContainer::ModifyQuery::Cursor &ShortTermContainer::ModifyQuery::Cursor::operator ++ () noexcept
{
    assert (container);
    assert (iterator != end);
    ++iterator;
    return *this;
}

ShortTermContainer::ModifyQuery::Cursor &ShortTermContainer::ModifyQuery::Cursor::operator ~ () noexcept
{
    assert (container);
    assert (iterator != end);

    container->pool.Release (*iterator);
    *iterator = *(end - 1u);

    container->objects.pop_back ();
    end = container->objects.end ();
    return *this;
}

ShortTermContainer::ModifyQuery::Cursor::Cursor (Handling::Handle <ShortTermContainer> _container) noexcept
    : container (std::move (_container)),
      iterator (container->objects.begin ()),
      end (container->objects.end ())
{
    assert (container);
    container->accessCounter.RegisterWriteAccess ();
}

ShortTermContainer::ModifyQuery::Cursor ShortTermContainer::ModifyQuery::Execute () const noexcept
{
    return Cursor (container);
}

ShortTermContainer::ModifyQuery::ModifyQuery (Handling::Handle <ShortTermContainer> _container) noexcept
    : container (std::move (_container))
{
    assert (container);
}

ShortTermContainer::InsertQuery ShortTermContainer::Insert () noexcept
{
    return InsertQuery (this);
}

ShortTermContainer::FetchQuery ShortTermContainer::Fetch () noexcept
{
    return FetchQuery (this);
}

ShortTermContainer::ModifyQuery ShortTermContainer::Modify () noexcept
{
    return ModifyQuery (this);
}

ShortTermContainer::ShortTermContainer (CargoDeck *_deck, StandardLayout::Mapping _typeMapping) noexcept
    : ContainerBase (_deck, std::move (_typeMapping)),
      objects (),
      pool (typeMapping.GetObjectSize ())
{
}

ShortTermContainer::~ShortTermContainer () noexcept
{
    assert (deck);
    deck->DetachContainer (this);
}
} // namespace Emergence::Galleon