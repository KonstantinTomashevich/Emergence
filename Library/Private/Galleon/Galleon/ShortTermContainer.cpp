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

void *ShortTermContainer::InsertQuery::Cursor::operator++ () noexcept
{
    assert (container);
    auto *node = static_cast<Node *> (container->pool.Acquire ());
    node->next = container->firstNode;
    container->firstNode = node;

    auto placeholder = container->pool.GetAllocationGroup ().PlaceOnTop ();
    container->typeMapping.Construct (&node->content);
    return &node->content;
}

ShortTermContainer::InsertQuery::Cursor::Cursor (Handling::Handle<ShortTermContainer> _container) noexcept
    : container (std::move (_container))
{
    assert (container);
    container->accessCounter.RegisterWriteAccess ();
}

ShortTermContainer::InsertQuery::Cursor ShortTermContainer::InsertQuery::Execute () const noexcept
{
    return Cursor (container);
}

Handling::Handle<ShortTermContainer> ShortTermContainer::InsertQuery::GetContainer () const noexcept
{
    assert (container);
    return container;
}

ShortTermContainer::InsertQuery::InsertQuery (Handling::Handle<ShortTermContainer> _container) noexcept
    : container (std::move (_container))
{
    assert (container);
}

ShortTermContainer::FetchQuery::Cursor::Cursor (const ShortTermContainer::FetchQuery::Cursor &_other) noexcept
    : container (_other.container),
      current (_other.current)
{
    assert (container);
    container->accessCounter.RegisterReadAccess ();
}

ShortTermContainer::FetchQuery::Cursor::Cursor (ShortTermContainer::FetchQuery::Cursor &&_other) noexcept
    : container (std::move (_other.container)),
      current (_other.current)
{
    _other.current = nullptr;
}

ShortTermContainer::FetchQuery::Cursor::~Cursor () noexcept
{
    if (container)
    {
        container->accessCounter.UnregisterReadAccess ();
    }
}

const void *ShortTermContainer::FetchQuery::Cursor::operator* () const noexcept
{
    assert (container);
    return current ? &current->content : nullptr;
}

ShortTermContainer::FetchQuery::Cursor &ShortTermContainer::FetchQuery::Cursor::operator++ () noexcept
{
    assert (container);
    assert (current);
    current = current->next;
    return *this;
}

ShortTermContainer::FetchQuery::Cursor::Cursor (Handling::Handle<ShortTermContainer> _container) noexcept
    : container (std::move (_container)),
      current (container->firstNode)
{
    assert (container);
    container->accessCounter.RegisterReadAccess ();
}

ShortTermContainer::FetchQuery::Cursor ShortTermContainer::FetchQuery::Execute () const noexcept
{
    return Cursor (container);
}

Handling::Handle<ShortTermContainer> ShortTermContainer::FetchQuery::GetContainer () const noexcept
{
    assert (container);
    return container;
}

ShortTermContainer::FetchQuery::FetchQuery (Handling::Handle<ShortTermContainer> _container) noexcept
    : container ((std::move (_container)))
{
    assert (container);
}

ShortTermContainer::ModifyQuery::Cursor::Cursor (ShortTermContainer::ModifyQuery::Cursor &&_other) noexcept
    : container (std::move (_other.container)),
      current (_other.current),
      previous (_other.previous)
{
    _other.current = nullptr;
    _other.previous = nullptr;
}

ShortTermContainer::ModifyQuery::Cursor::~Cursor () noexcept
{
    if (container)
    {
        container->accessCounter.UnregisterWriteAccess ();
    }
}

void *ShortTermContainer::ModifyQuery::Cursor::operator* () noexcept
{
    assert (container);
    return current ? &current->content : nullptr;
}

ShortTermContainer::ModifyQuery::Cursor &ShortTermContainer::ModifyQuery::Cursor::operator++ () noexcept
{
    assert (container);
    assert (current);

    previous = current;
    current = current->next;
    return *this;
}

ShortTermContainer::ModifyQuery::Cursor &ShortTermContainer::ModifyQuery::Cursor::operator~ () noexcept
{
    assert (container);
    assert (current);

    Node *next = current->next;
    container->typeMapping.Destruct (&current->content);
    container->pool.Release (current);
    current = next;

    if (previous)
    {
        previous->next = next;
    }
    else
    {
        container->firstNode = next;
    }

    return *this;
}

ShortTermContainer::ModifyQuery::Cursor::Cursor (Handling::Handle<ShortTermContainer> _container) noexcept
    : container (std::move (_container)),
      current (container->firstNode)
{
    assert (container);
    container->accessCounter.RegisterWriteAccess ();
}

ShortTermContainer::ModifyQuery::Cursor ShortTermContainer::ModifyQuery::Execute () const noexcept
{
    return Cursor (container);
}

Handling::Handle<ShortTermContainer> ShortTermContainer::ModifyQuery::GetContainer () const noexcept
{
    assert (container);
    return container;
}

ShortTermContainer::ModifyQuery::ModifyQuery (Handling::Handle<ShortTermContainer> _container) noexcept
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

void ShortTermContainer::LastReferenceUnregistered () noexcept
{
    assert (deck);
    deck->DetachContainer (this);
}

ShortTermContainer::ShortTermContainer (CargoDeck *_deck, StandardLayout::Mapping _typeMapping) noexcept
    : ContainerBase (_deck, std::move (_typeMapping)),
      pool (Memory::Profiler::AllocationGroup {Memory::UniqueString {typeMapping.GetName ()}},
            sizeof (Node) + typeMapping.GetObjectSize ())
{
}

ShortTermContainer::~ShortTermContainer () noexcept
{
    Node *node = firstNode;
    while (node)
    {
        typeMapping.Destruct (&node->content);
        node = node->next;
    }
}
} // namespace Emergence::Galleon
