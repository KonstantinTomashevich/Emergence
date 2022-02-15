#pragma once

#include <array>
#include <cassert>
#include <concepts>

namespace Emergence::Container
{
// TODO: Add tests for InplaceVector?

/// \brief Allows to use std::array as inplace vector with fixed capacity.
template <typename Item, std::size_t Capacity>
class InplaceVector
{
public:
    using ValueType = Item;

    using value_type = ValueType;

    using Iterator = typename std::array<Item, Capacity>::iterator;

    using ConstIterator = typename std::array<Item, Capacity>::const_iterator;

    /// \brief Constructs empty inplace vector without initializing reserved memory.
    InplaceVector () noexcept;

    /// \brief Copies values from given inplace vector.
    InplaceVector (const InplaceVector &_other) noexcept;

    /// \brief Moves values from given inplace vector and leaves it empty.
    InplaceVector (InplaceVector &&_other) noexcept;

    InplaceVector (std::initializer_list<Item> _initializer) noexcept;

    ~InplaceVector () noexcept;

    Iterator Begin () noexcept;

    Iterator End () noexcept;

    [[nodiscard]] ConstIterator Begin () const noexcept;

    [[nodiscard]] ConstIterator End () const noexcept;

    /// \return ::count.
    [[nodiscard]] std::size_t GetCount () const noexcept;

    /// \return Capacity
    [[nodiscard]] std::size_t GetCapacity () const noexcept;

    /// \return Is vector empty?
    [[nodiscard]] bool Empty () const noexcept;

    /// \brief Constructs new item in next free slot.
    /// \return Reference to constructed item.
    /// \invariant ::GetCount is less that ::Capacity.
    template <typename... Args>
    Item &EmplaceBack (Args &&..._constructorArgs) noexcept;

    /// \brief Constructs new item in next free slot if there is any space left.
    /// \return Whether item was constructed.
    template <typename... Args>
    bool TryEmplaceBack (Args &&..._constructorArgs) noexcept;

    /// \brief Shifts all elements starting from `_at` by one and constructs new item at `_at`.
    /// \return Reference to constructed item.
    /// \invariant ::GetCount is less that ::Capacity.
    template <typename... Args>
    Item &EmplaceAt (Iterator _at,
                     Args &&..._constructorArgs) noexcept requires std::is_nothrow_move_assignable_v<Item>;

    /// \brief Resets vector to empty state.
    void Clear () noexcept;

    /// \brief Move assigns last value to position of given iterator and decrements ::count.
    /// \return New ::End iterator.
    /// \invariant _iterator < ::End.
    Iterator EraseExchangingWithLast (
        const Iterator &_iterator) noexcept requires std::is_nothrow_move_assignable_v<Item>;

    /// \brief Erase first `_amount` elements **without** preserving element order (for performance).
    /// \invariant _amount <= ::GetCount.
    void DropLeading (std::size_t _amount) noexcept;

    /// \brief Erases last `_amount` elements.
    /// \invariant _amount <= ::GetCount.
    void DropTrailing (std::size_t _amount) noexcept;

    /// \return First item in vector.
    /// \invariant Not ::Empty.
    [[nodiscard]] Item &Front () noexcept;

    /// \return First item in vector.
    /// \invariant Not ::Empty.
    [[nodiscard]] const Item &Front () const noexcept;

    /// \return Last item in vector.
    /// \invariant Not ::Empty.
    [[nodiscard]] Item &Back () noexcept;

    /// \return Last item in vector.
    /// \invariant Not ::Empty.
    [[nodiscard]] const Item &Back () const noexcept;

    /// \return Removes last item from vector.
    /// \invariant Not ::Empty.
    void PopBack () noexcept;

    /// \brief Shortcut for std::find on this vector.
    [[nodiscard]] Iterator Find (const Item &_item) noexcept;

    /// \brief Shortcut for std::find on this vector.
    [[nodiscard]] ConstIterator Find (const Item &_item) const noexcept;

    bool operator== (const InplaceVector &_other) const noexcept requires std::equality_comparable<Item>;

    bool operator!= (const InplaceVector &_other) const noexcept;

    InplaceVector &operator= (const InplaceVector &_other) noexcept;

    InplaceVector &operator= (InplaceVector &&_other) noexcept;

    const Item &operator[] (std::size_t _index) const noexcept;

    Item &operator[] (std::size_t _index) noexcept;

private:
    std::size_t count {0u};

    /// \details We don't want to initialize all data right away, therefore we
    ///          are using union to get rid of array constructor and destructor.
    union
    {
        std::array<Item, Capacity> values;
    };
};

template <typename Item, std::size_t Capacity>
InplaceVector<Item, Capacity>::InplaceVector () noexcept
{
}

template <typename Item, std::size_t Capacity>
InplaceVector<Item, Capacity>::InplaceVector (const InplaceVector &_other) noexcept : count (_other.count)
{
    for (std::size_t index = 0u; index < count; ++index)
    {
        new (&values[index]) Item (_other.values[index]);
    }
}

template <typename Item, std::size_t Capacity>
InplaceVector<Item, Capacity>::InplaceVector (InplaceVector &&_other) noexcept : count (_other.count)
{
    for (std::size_t index = 0u; index < count; ++index)
    {
        new (&values[index]) Item (std::move (_other.values[index]));
    }

    _other.Clear ();
}

template <typename Item, std::size_t Capacity>
InplaceVector<Item, Capacity>::InplaceVector (std::initializer_list<Item> _initializer) noexcept
    : count (_initializer.size ())
{
    assert (count <= Capacity);
    for (std::size_t index = 0u; index < count; ++index)
    {
        new (&values[index]) Item (std::move (data (_initializer)[index]));
    }
}

template <typename Item, std::size_t Capacity>
InplaceVector<Item, Capacity>::~InplaceVector () noexcept
{
    Clear ();
}

template <typename Item, std::size_t Capacity>
typename InplaceVector<Item, Capacity>::Iterator InplaceVector<Item, Capacity>::Begin () noexcept
{
    return values.begin ();
}

template <typename Item, std::size_t Capacity>
typename InplaceVector<Item, Capacity>::ConstIterator InplaceVector<Item, Capacity>::Begin () const noexcept
{
    return values.begin ();
}

template <typename Item, std::size_t Capacity>
typename InplaceVector<Item, Capacity>::ConstIterator InplaceVector<Item, Capacity>::End () const noexcept
{
    return values.begin () + count;
}

template <typename Item, std::size_t Capacity>
typename InplaceVector<Item, Capacity>::Iterator InplaceVector<Item, Capacity>::End () noexcept
{
    return values.begin () + count;
}

template <typename Item, std::size_t Capacity>
std::size_t InplaceVector<Item, Capacity>::GetCount () const noexcept
{
    return count;
}

template <typename Item, std::size_t Capacity>
std::size_t InplaceVector<Item, Capacity>::GetCapacity () const noexcept
{
    return Capacity;
}

template <typename Item, std::size_t Capacity>
bool InplaceVector<Item, Capacity>::Empty () const noexcept
{
    return count == 0u;
}

template <typename Item, std::size_t Capacity>
template <typename... Args>
Item &InplaceVector<Item, Capacity>::EmplaceBack (Args &&..._constructorArgs) noexcept
{
    assert (count < Capacity);
    Item *item = new (&values[count]) Item (std::forward<Args> (_constructorArgs)...);
    ++count;
    return *item;
}

template <typename Item, std::size_t Capacity>
template <typename... Args>
bool InplaceVector<Item, Capacity>::TryEmplaceBack (Args &&..._constructorArgs) noexcept
{
    if (count < Capacity)
    {
        EmplaceBack (std::forward<Args...> (_constructorArgs...));
        return true;
    }

    return false;
}

template <typename Item, std::size_t Capacity>
template <typename... Args>
Item &InplaceVector<Item, Capacity>::EmplaceAt (
    InplaceVector::Iterator _at, Args &&..._constructorArgs) noexcept requires std::is_nothrow_move_assignable_v<Item>
{
    assert (count < Capacity);
    if (_at == End ())
    {
        return EmplaceBack (std::forward<Args...> (_constructorArgs...));
    }

    const auto last = --End ();
    new (&*End ()) Item {std::move (*last)};

    for (auto iterator = last; iterator != _at; --iterator)
    {
        *iterator = std::move (*(iterator - 1u));
    }

    _at->~Item ();
    new (&*_at) Item {std::forward<Args> (_constructorArgs)...};
    ++count;
    return *_at;
}

template <typename Item, std::size_t Capacity>
void InplaceVector<Item, Capacity>::Clear () noexcept
{
    count = 0u;

    // There is no sense to clear trivial items.
    if constexpr (!std::is_trivial_v<Item>)
    {
        for (std::size_t index = 0u; index < count; ++index)
        {
            values[index].~Item ();
        }
    }
}

template <typename Item, std::size_t Capacity>
typename InplaceVector<Item, Capacity>::Iterator InplaceVector<Item, Capacity>::EraseExchangingWithLast (
    const InplaceVector::Iterator &_iterator) noexcept requires std::is_nothrow_move_assignable_v<Item>
{
    assert (_iterator < End ());
    auto last = --End ();

    if (_iterator != last)
    {
        *_iterator = std::move (*last);
    }

    last->~Item ();
    --count;
    return _iterator;
}

template <typename Item, std::size_t Capacity>
void InplaceVector<Item, Capacity>::DropLeading (std::size_t _amount) noexcept
{
    assert (_amount <= count);
    if (_amount >= count)
    {
        Clear ();
        return;
    }

    auto iterator = Begin ();
    const auto dropEnd = iterator + _amount;

    while (true)
    {
        if (iterator == dropEnd)
        {
            // We have dropped required amount by exchanging.
            break;
        }

        if (dropEnd == End ())
        {
            // There is no sense to exchange anymore, because all items above must be dropped.
            DropTrailing (dropEnd - iterator);
            break;
        }

        iterator = EraseExchangingWithLast (iterator);
        ++iterator;
    }
}

template <typename Item, std::size_t Capacity>
void InplaceVector<Item, Capacity>::DropTrailing (std::size_t _amount) noexcept
{
    assert (_amount <= count);
    if (_amount >= count)
    {
        Clear ();
        return;
    }

    // There is no sense to clear trivial items.
    if constexpr (!std::is_trivial_v<Item>)
    {
        auto iterator = End () - _amount;
        while (iterator != End ())
        {
            iterator->~Item ();
            ++iterator;
        }
    }

    count -= _amount;
}

template <typename Item, std::size_t Capacity>
Item &InplaceVector<Item, Capacity>::Front () noexcept
{
    assert (!Empty ());
    return values[0u];
}

template <typename Item, std::size_t Capacity>
const Item &InplaceVector<Item, Capacity>::Front () const noexcept
{
    assert (!Empty ());
    return values[0u];
}

template <typename Item, std::size_t Capacity>
Item &InplaceVector<Item, Capacity>::Back () noexcept
{
    assert (!Empty ());
    return values[count - 1u];
}

template <typename Item, std::size_t Capacity>
const Item &InplaceVector<Item, Capacity>::Back () const noexcept
{
    assert (!Empty ());
    return values[count - 1u];
}

template <typename Item, std::size_t Capacity>
void InplaceVector<Item, Capacity>::PopBack () noexcept
{
    assert (!Empty ());
    Back ().~Item ();
    --count;
}

template <typename Item, std::size_t Capacity>
typename InplaceVector<Item, Capacity>::Iterator InplaceVector<Item, Capacity>::Find (const Item &_item) noexcept
{
    return std::find (Begin (), End (), _item);
}

template <typename Item, std::size_t Capacity>
typename InplaceVector<Item, Capacity>::ConstIterator InplaceVector<Item, Capacity>::Find (
    const Item &_item) const noexcept
{
    return std::find (Begin (), End (), _item);
}

template <typename Item, std::size_t Capacity>
bool InplaceVector<Item, Capacity>::operator== (
    const InplaceVector &_other) const noexcept requires std::equality_comparable<Item>
{
    if (count != _other.count)
    {
        return false;
    }

    auto myIterator = Begin ();
    const auto myEnd = End ();
    auto otherIterator = _other.Begin ();

    while (myIterator != myEnd)
    {
        if (*myIterator != *otherIterator)
        {
            return false;
        }

        ++myIterator;
        ++otherIterator;
    }

    return true;
}

template <typename Item, std::size_t Capacity>
bool InplaceVector<Item, Capacity>::operator!= (const InplaceVector &_other) const noexcept
{
    return !(_other == *this);
}

template <typename Item, std::size_t Capacity>
InplaceVector<Item, Capacity> &InplaceVector<Item, Capacity>::operator= (const InplaceVector &_other) noexcept
{
    if (this != &_other)
    {
        this->~InplaceVector ();
        new (this) InplaceVector (_other);
    }

    return *this;
}

template <typename Item, std::size_t Capacity>
InplaceVector<Item, Capacity> &InplaceVector<Item, Capacity>::operator= (InplaceVector &&_other) noexcept
{
    if (this != &_other)
    {
        this->~InplaceVector ();
        new (this) InplaceVector (std::move (_other));
    }

    return *this;
}

template <typename Item, std::size_t Capacity>
const Item &InplaceVector<Item, Capacity>::operator[] (std::size_t _index) const noexcept
{
    assert (_index < GetCount ());
    return values[_index];
}

template <typename Item, std::size_t Capacity>
Item &InplaceVector<Item, Capacity>::operator[] (std::size_t _index) noexcept
{
    assert (_index < GetCount ());
    return values[_index];
}

template <typename Item, std::size_t Capacity>
typename InplaceVector<Item, Capacity>::Iterator begin (InplaceVector<Item, Capacity> &_vector) noexcept
{
    return _vector.Begin ();
}

template <typename Item, std::size_t Capacity>
typename InplaceVector<Item, Capacity>::Iterator end (InplaceVector<Item, Capacity> &_vector) noexcept
{
    return _vector.End ();
}

template <typename Item, std::size_t Capacity>
typename InplaceVector<Item, Capacity>::ConstIterator begin (const InplaceVector<Item, Capacity> &_vector) noexcept
{
    return _vector.Begin ();
}

template <typename Item, std::size_t Capacity>
typename InplaceVector<Item, Capacity>::ConstIterator end (const InplaceVector<Item, Capacity> &_vector) noexcept
{
    return _vector.End ();
}
} // namespace Emergence::Container
