#include <Assert/Assert.hpp>

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
    EMERGENCE_ASSERT (container);
    void *node = container->pool.Acquire ();
    container->SetNextNode (node, container->firstNode);
    container->firstNode = node;

    auto placeholder = container->pool.GetAllocationGroup ().PlaceOnTop ();
    container->typeMapping.Construct (ShortTermContainer::GetNodeContent (node));
    return ShortTermContainer::GetNodeContent (node);
}

ShortTermContainer::InsertQuery::Cursor::Cursor (Handling::Handle<ShortTermContainer> _container) noexcept
    : container (std::move (_container))
{
    EMERGENCE_ASSERT (container);
    container->accessCounter.RegisterWriteAccess ();
}

ShortTermContainer::InsertQuery::Cursor ShortTermContainer::InsertQuery::Execute () const noexcept
{
    return Cursor (container);
}

Handling::Handle<ShortTermContainer> ShortTermContainer::InsertQuery::GetContainer () const noexcept
{
    EMERGENCE_ASSERT (container);
    return container;
}

ShortTermContainer::InsertQuery::InsertQuery (Handling::Handle<ShortTermContainer> _container) noexcept
    : container (std::move (_container))
{
    EMERGENCE_ASSERT (container);
}

ShortTermContainer::FetchQuery::Cursor::Cursor (const ShortTermContainer::FetchQuery::Cursor &_other) noexcept
    : container (_other.container),
      currentNode (_other.currentNode)
{
    EMERGENCE_ASSERT (container);
    container->accessCounter.RegisterReadAccess ();
}

ShortTermContainer::FetchQuery::Cursor::Cursor (ShortTermContainer::FetchQuery::Cursor &&_other) noexcept
    : container (std::move (_other.container)),
      currentNode (_other.currentNode)
{
    _other.currentNode = nullptr;
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
    EMERGENCE_ASSERT (container);
    return ShortTermContainer::GetNodeContent (currentNode);
}

ShortTermContainer::FetchQuery::Cursor &ShortTermContainer::FetchQuery::Cursor::operator++ () noexcept
{
    EMERGENCE_ASSERT (container);
    EMERGENCE_ASSERT (currentNode);
    currentNode = container->GetNextNode (currentNode);
    return *this;
}

ShortTermContainer::FetchQuery::Cursor::Cursor (Handling::Handle<ShortTermContainer> _container) noexcept
    : container (std::move (_container)),
      currentNode (container->firstNode)
{
    EMERGENCE_ASSERT (container);
    container->accessCounter.RegisterReadAccess ();
}

ShortTermContainer::FetchQuery::Cursor ShortTermContainer::FetchQuery::Execute () const noexcept
{
    return Cursor (container);
}

Handling::Handle<ShortTermContainer> ShortTermContainer::FetchQuery::GetContainer () const noexcept
{
    EMERGENCE_ASSERT (container);
    return container;
}

ShortTermContainer::FetchQuery::FetchQuery (Handling::Handle<ShortTermContainer> _container) noexcept
    : container ((std::move (_container)))
{
    EMERGENCE_ASSERT (container);
}

ShortTermContainer::ModifyQuery::Cursor::Cursor (ShortTermContainer::ModifyQuery::Cursor &&_other) noexcept
    : container (std::move (_other.container)),
      currentNode (_other.currentNode),
      previousNode (_other.previousNode)
{
    _other.currentNode = nullptr;
    _other.previousNode = nullptr;
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
    EMERGENCE_ASSERT (container);
    return currentNode ? ShortTermContainer::GetNodeContent (currentNode) : nullptr;
}

ShortTermContainer::ModifyQuery::Cursor &ShortTermContainer::ModifyQuery::Cursor::operator++ () noexcept
{
    EMERGENCE_ASSERT (container);
    EMERGENCE_ASSERT (currentNode);

    previousNode = currentNode;
    currentNode = container->GetNextNode (currentNode);
    return *this;
}

ShortTermContainer::ModifyQuery::Cursor &ShortTermContainer::ModifyQuery::Cursor::operator~() noexcept
{
    EMERGENCE_ASSERT (container);
    EMERGENCE_ASSERT (currentNode);

    void *next = container->GetNextNode (currentNode);
    container->typeMapping.Destruct (ShortTermContainer::GetNodeContent (currentNode));
    container->pool.Release (currentNode);
    currentNode = next;

    if (previousNode)
    {
        container->SetNextNode (previousNode, next);
    }
    else
    {
        container->firstNode = next;
    }

    return *this;
}

ShortTermContainer::ModifyQuery::Cursor::Cursor (Handling::Handle<ShortTermContainer> _container) noexcept
    : container (std::move (_container)),
      currentNode (container->firstNode)
{
    EMERGENCE_ASSERT (container);
    container->accessCounter.RegisterWriteAccess ();
}

ShortTermContainer::ModifyQuery::Cursor ShortTermContainer::ModifyQuery::Execute () const noexcept
{
    return Cursor (container);
}

Handling::Handle<ShortTermContainer> ShortTermContainer::ModifyQuery::GetContainer () const noexcept
{
    EMERGENCE_ASSERT (container);
    return container;
}

ShortTermContainer::ModifyQuery::ModifyQuery (Handling::Handle<ShortTermContainer> _container) noexcept
    : container (std::move (_container))
{
    EMERGENCE_ASSERT (container);
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
    EMERGENCE_ASSERT (deck);
    deck->DetachContainer (this);
}

void ShortTermContainer::SetUnsafeFetchAllowed (bool _allowed) noexcept
{
    accessCounter.SetUnsafeFetchAllowed (_allowed);
}

void *ShortTermContainer::GetNodeContent (void *_node) noexcept
{
    return _node;
}

const void *ShortTermContainer::GetNodeContent (const void *_node) noexcept
{
    return _node;
}

ShortTermContainer::ShortTermContainer (CargoDeck *_deck, StandardLayout::Mapping _typeMapping) noexcept
    : ContainerBase (_deck, std::move (_typeMapping)),
      pool (Memory::Profiler::AllocationGroup {Memory::UniqueString {typeMapping.GetName ()}},
            typeMapping.GetObjectSize () + sizeof (uintptr_t),
            typeMapping.GetObjectAlignment ())
{
    EMERGENCE_ASSERT (typeMapping.GetObjectSize () % sizeof (uintptr_t) == 0u);
}

ShortTermContainer::~ShortTermContainer () noexcept
{
    void *node = firstNode;
    while (node)
    {
        typeMapping.Destruct (GetNodeContent (node));
        node = GetNextNode (node);
    }
}

const void *ShortTermContainer::GetNextNode (const void *_node) noexcept
{
    return *reinterpret_cast<void *const *> (static_cast<const uint8_t *> (_node) + typeMapping.GetObjectSize ());
}

void *ShortTermContainer::GetNextNode (void *_node) noexcept
{
    return const_cast<void *> (GetNextNode (const_cast<const void *> (_node)));
}

void ShortTermContainer::SetNextNode (void *_node, void *_next) noexcept
{
    *reinterpret_cast<void **> (static_cast<uint8_t *> (_node) + typeMapping.GetObjectSize ()) = _next;
}
} // namespace Emergence::Galleon
