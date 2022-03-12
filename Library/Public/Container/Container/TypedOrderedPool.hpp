#pragma once

#include <Memory/OrderedPool.hpp>

namespace Emergence::Container
{
/// \brief Wrapper for Memory::OrderedPool, that associates Item type with wrapped pool.
/// \details Sometimes we need to store large amount of items and rarely iterate over them.
///          In this case Vector is not ideal solution, because it takes one very large block of memory due to
///          memory continuity constraint. OrderedPool is better suited for such situations, because it both allows
///          objects to be stored in different blocks of memory and provides relatively fast iteration over them.
template <typename Item>
class TypedOrderedPool final
{
public:
    /// \brief Allows const iteration over acquired items.
    class AcquiredConstIterator final
    {
    public:
        EMERGENCE_FORWARD_ITERATOR_OPERATIONS (AcquiredConstIterator, const Item &);

    private:
        /// Pool constructs iterators.
        friend class TypedOrderedPool<Item>;

        AcquiredConstIterator (Memory::OrderedPool::AcquiredChunkConstIterator _iterator) noexcept;

        Memory::OrderedPool::AcquiredChunkConstIterator iterator;
    };

    /// \brief Allows iteration over acquired items.
    class AcquiredIterator final
    {
    public:
        EMERGENCE_FORWARD_ITERATOR_OPERATIONS (AcquiredIterator, Item &);

    private:
        /// Pool constructs iterators.
        friend class TypedOrderedPool<Item>;

        AcquiredIterator (Memory::OrderedPool::AcquiredChunkIterator _iterator) noexcept;

        Memory::OrderedPool::AcquiredChunkIterator iterator;
    };

    TypedOrderedPool (Memory::Profiler::AllocationGroup _group) noexcept;

    TypedOrderedPool (const TypedOrderedPool &_other) = delete;

    TypedOrderedPool (TypedOrderedPool &&_other) noexcept = default;

    ~TypedOrderedPool () noexcept;

    /// \brief Acquires memory and constructs new item.
    template <typename... Arguments>
    Item &Acquire (Arguments &&..._arguments) noexcept;

    /// \brief Destructs given item and releases its memory.
    void Release (Item &_item) noexcept;

    /// \see OrderedPool::Shrink
    void Shrink () noexcept;

    /// \see OrderedPool::Clear
    void Clear () noexcept;

    [[nodiscard]] AcquiredConstIterator Begin () const noexcept;

    [[nodiscard]] AcquiredConstIterator End () const noexcept;

    [[nodiscard]] AcquiredIterator Begin () noexcept;

    [[nodiscard]] AcquiredIterator End () noexcept;

    TypedOrderedPool &operator= (const TypedOrderedPool &_other) = delete;

    TypedOrderedPool &operator= (TypedOrderedPool &&_other) noexcept;

private:
    Memory::OrderedPool pool;
};

/// \brief Wraps TypedOrderedPool::Begin for foreach sentences.
template <typename Item>
typename TypedOrderedPool<Item>::AcquiredConstIterator begin (const TypedOrderedPool<Item> &_pool) noexcept
{
    return _pool.Begin ();
}

/// \brief Wraps TypedOrderedPool<Item>::End for foreach sentences.
template <typename Item>
typename TypedOrderedPool<Item>::AcquiredConstIterator end (const TypedOrderedPool<Item> &_pool) noexcept
{
    return _pool.End ();
}

/// \brief Wraps TypedOrderedPool<Item>::Begin for foreach sentences.
template <typename Item>
typename TypedOrderedPool<Item>::AcquiredIterator begin (TypedOrderedPool<Item> &_pool) noexcept
{
    return _pool.Begin ();
}

/// \brief Wraps TypedOrderedPool<Item>::End for foreach sentences.
template <typename Item>
typename TypedOrderedPool<Item>::AcquiredIterator end (TypedOrderedPool<Item> &_pool) noexcept
{
    return _pool.End ();
}

template <typename Item>
const Item &TypedOrderedPool<Item>::AcquiredConstIterator::operator* () const noexcept
{
    return *static_cast<const Item *> (*iterator);
}

template <typename Item>
typename TypedOrderedPool<Item>::AcquiredConstIterator &TypedOrderedPool<Item>::AcquiredConstIterator::operator= (
    const TypedOrderedPool::AcquiredConstIterator &_other) noexcept = default;

template <typename Item>
typename TypedOrderedPool<Item>::AcquiredConstIterator &TypedOrderedPool<Item>::AcquiredConstIterator::operator= (
    TypedOrderedPool::AcquiredConstIterator &&_other) noexcept = default;

template <typename Item>
TypedOrderedPool<Item>::AcquiredConstIterator::~AcquiredConstIterator () noexcept = default;

template <typename Item>
typename TypedOrderedPool<Item>::AcquiredConstIterator &
TypedOrderedPool<Item>::AcquiredConstIterator::operator++ () noexcept
{
    ++iterator;
    return *this;
}

template <typename Item>
typename TypedOrderedPool<Item>::AcquiredConstIterator TypedOrderedPool<Item>::AcquiredConstIterator::operator++ (
    int) noexcept
{
    AcquiredConstIterator copy = *this;
    ++*this;
    return copy;
}

template <typename Item>
TypedOrderedPool<Item>::AcquiredConstIterator::AcquiredConstIterator (
    const TypedOrderedPool::AcquiredConstIterator &_other) noexcept = default;

template <typename Item>
TypedOrderedPool<Item>::AcquiredConstIterator::AcquiredConstIterator (
    TypedOrderedPool::AcquiredConstIterator &&_other) noexcept = default;

template <typename Item>
bool TypedOrderedPool<Item>::AcquiredConstIterator::operator== (
    const TypedOrderedPool::AcquiredConstIterator &_other) const noexcept
{
    return iterator == _other.iterator;
}

template <typename Item>
bool TypedOrderedPool<Item>::AcquiredConstIterator::operator!= (
    const TypedOrderedPool::AcquiredConstIterator &_other) const noexcept
{
    return !(*this == _other);
}

template <typename Item>
TypedOrderedPool<Item>::AcquiredConstIterator::AcquiredConstIterator (
    Memory::OrderedPool::AcquiredChunkConstIterator _iterator) noexcept
    : iterator (std::move (_iterator))
{
}

template <typename Item>
Item &TypedOrderedPool<Item>::AcquiredIterator::operator* () const noexcept
{
    return *static_cast<Item *> (*iterator);
}

template <typename Item>
typename TypedOrderedPool<Item>::AcquiredIterator &TypedOrderedPool<Item>::AcquiredIterator::operator= (
    const TypedOrderedPool::AcquiredIterator &_other) noexcept = default;

template <typename Item>
typename TypedOrderedPool<Item>::AcquiredIterator &TypedOrderedPool<Item>::AcquiredIterator::operator= (
    TypedOrderedPool::AcquiredIterator &&_other) noexcept = default;

template <typename Item>
TypedOrderedPool<Item>::AcquiredIterator::~AcquiredIterator () noexcept = default;

template <typename Item>
typename TypedOrderedPool<Item>::AcquiredIterator &TypedOrderedPool<Item>::AcquiredIterator::operator++ () noexcept
{
    ++iterator;
    return *this;
}

template <typename Item>
typename TypedOrderedPool<Item>::AcquiredIterator TypedOrderedPool<Item>::AcquiredIterator::operator++ (int) noexcept
{
    AcquiredIterator copy = *this;
    ++*this;
    return copy;
}

template <typename Item>
TypedOrderedPool<Item>::AcquiredIterator::AcquiredIterator (const TypedOrderedPool::AcquiredIterator &_other) noexcept =
    default;

template <typename Item>
TypedOrderedPool<Item>::AcquiredIterator::AcquiredIterator (TypedOrderedPool::AcquiredIterator &&_other) noexcept =
    default;

template <typename Item>
bool TypedOrderedPool<Item>::AcquiredIterator::operator== (
    const TypedOrderedPool::AcquiredIterator &_other) const noexcept
{
    return iterator == _other.iterator;
}

template <typename Item>
bool TypedOrderedPool<Item>::AcquiredIterator::operator!= (
    const TypedOrderedPool::AcquiredIterator &_other) const noexcept
{
    return !(*this == _other);
}

template <typename Item>
TypedOrderedPool<Item>::AcquiredIterator::AcquiredIterator (
    Memory::OrderedPool::AcquiredChunkIterator _iterator) noexcept
    : iterator (std::move (_iterator))
{
}

template <typename Item>
TypedOrderedPool<Item>::TypedOrderedPool (Memory::Profiler::AllocationGroup _group) noexcept
    : pool (std::move (_group), sizeof (Item))
{
}

template <typename Item>
TypedOrderedPool<Item>::~TypedOrderedPool () noexcept
{
    Clear ();
}

template <typename Item>
template <typename... Arguments>
Item &TypedOrderedPool<Item>::Acquire (Arguments &&..._arguments) noexcept
{
    return *new (pool.Acquire ()) Item (std::forward<Arguments> (_arguments)...);
}

template <typename Item>
void TypedOrderedPool<Item>::Release (Item &_item) noexcept
{
    _item.~Item ();
    pool.Release (&_item);
}

template <typename Item>
void TypedOrderedPool<Item>::Shrink () noexcept
{
    pool.Shrink ();
}

template <typename Item>
void TypedOrderedPool<Item>::Clear () noexcept
{
    for (Item &item : *this)
    {
        item.~Item ();
    }

    pool.Clear ();
}

template <typename Item>
typename TypedOrderedPool<Item>::AcquiredConstIterator TypedOrderedPool<Item>::Begin () const noexcept
{
    return pool.BeginAcquired ();
}

template <typename Item>
typename TypedOrderedPool<Item>::AcquiredConstIterator TypedOrderedPool<Item>::End () const noexcept
{
    return pool.EndAcquired ();
}

template <typename Item>
typename TypedOrderedPool<Item>::AcquiredIterator TypedOrderedPool<Item>::Begin () noexcept
{
    return pool.BeginAcquired ();
}

template <typename Item>
typename TypedOrderedPool<Item>::AcquiredIterator TypedOrderedPool<Item>::End () noexcept
{
    return pool.EndAcquired ();
}

template <typename Item>
TypedOrderedPool<Item> &TypedOrderedPool<Item>::operator= (TypedOrderedPool &&_other) noexcept
{
    if (this != &_other)
    {
        this->~TypedOrderedPool<Item> ();
        new (this) TypedOrderedPool<Item> (std::move (_other));
    }

    return *this;
}
} // namespace Emergence::Container
