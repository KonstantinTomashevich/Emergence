#pragma once

#include <array>
#include <cassert>
#include <concepts>

namespace Emergence
{
// TODO: Add tests for InplaceVector?

/// \brief Allows to use std::array as inplace vector with fixed capacity.
template <typename Item, std::size_t Capacity>
class InplaceVector
{
public:
    using Iterator = typename std::array<Item, Capacity>::iterator;

    using ConstIterator = typename std::array<Item, Capacity>::const_iterator;

    /// \brief Constructs empty inplace vector.
    /// \warning Default constructor called for all items in ::values, because std::array is base container.
    InplaceVector () noexcept requires std::is_nothrow_default_constructible_v<Item>
    = default;

    /// \brief Copies values from given inplace vector.
    InplaceVector (const InplaceVector &_other) noexcept requires std::is_nothrow_copy_constructible_v<Item>;

    /// \brief Moves values from given inplace vector and leaves it empty.
    InplaceVector (InplaceVector &&_other) noexcept requires std::is_nothrow_move_constructible_v<Item>;

    InplaceVector (std::initializer_list<Item> _initializer) noexcept requires std::is_nothrow_move_assignable_v<Item>;

    ~InplaceVector () noexcept = default;

    Iterator Begin () noexcept;

    Iterator End () noexcept;

    [[nodiscard]] ConstIterator Begin () const noexcept;

    [[nodiscard]] ConstIterator End () const noexcept;

    /// \return ::count.
    [[nodiscard]] std::size_t GetCount () const noexcept;

    /// \return is vector empty?
    [[nodiscard]] bool Empty () const noexcept;

    /// \brief Constructs new item in next free slot.
    /// \return Reference to constructed item.
    /// \invariant ::GetCount is less that ::Capacity.
    template <typename... Args>
    Item &EmplaceBack (Args &&..._constructorArgs) noexcept;

    /// \brief Resets vector to empty state.
    void Clear () noexcept requires std::is_nothrow_default_constructible_v<Item>;

    /// \brief Move assigns last value to position of given iterator and decrements ::count.
    /// \return New ::End iterator.
    /// \invariant _iterator < ::End.
    Iterator EraseExchangingWithLast (const Iterator &_iterator) noexcept requires
        std::is_nothrow_move_assignable_v<Item> && std::is_nothrow_default_constructible_v<Item>;

    bool operator== (const InplaceVector &_other) const noexcept requires std::equality_comparable<Item>;

    bool operator!= (const InplaceVector &_other) const noexcept;

    InplaceVector &operator= (const InplaceVector &_other) noexcept;

    InplaceVector &operator= (InplaceVector &&_other) noexcept;

    const Item &operator[] (std::size_t _index) const noexcept;

    Item &operator[] (std::size_t _index) noexcept;

private:
    std::size_t count {0u};
    std::array<Item, Capacity> values {};
};

template <typename Item, std::size_t Capacity>
InplaceVector<Item, Capacity>::InplaceVector (
    const InplaceVector &_other) noexcept requires std::is_nothrow_copy_constructible_v<Item> : count (_other.count),
                                                                                                values (_other.values)
{
}

template <typename Item, std::size_t Capacity>
typename InplaceVector<Item, Capacity>::Iterator InplaceVector<Item, Capacity>::Begin () noexcept
{
    return values.begin ();
}

template <typename Item, std::size_t Capacity>
InplaceVector<Item, Capacity>::InplaceVector (
    InplaceVector &&_other) noexcept requires std::is_nothrow_move_constructible_v<Item>
    : count (_other.count), values (std::move (_other.values))
{
    _other.Clear ();
}

template <typename Item, std::size_t Capacity>
InplaceVector<Item, Capacity>::InplaceVector (
    std::initializer_list<Item> _initializer) noexcept requires std::is_nothrow_move_assignable_v<Item>
    : count (_initializer.size ()), values ()
{
    assert (count <= Capacity);
    for (std::size_t index = 0u; index < count; ++index)
    {
        values[index] = std::move (data (_initializer)[index]);
    }
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
bool InplaceVector<Item, Capacity>::Empty () const noexcept
{
    return count == 0u;
}

template <typename Item, std::size_t Capacity>
template <typename... Args>
Item &InplaceVector<Item, Capacity>::EmplaceBack (Args &&..._constructorArgs) noexcept
{
    assert (count < Capacity);
    values[count].~Item ();
    Item *item = new (&values[count]) Item (std::forward<Args> (_constructorArgs)...);
    ++count;
    return *item;
}

template <typename Item, std::size_t Capacity>
void InplaceVector<Item, Capacity>::Clear () noexcept requires std::is_nothrow_default_constructible_v<Item>
{
    count = 0u;
    values = {};
}

template <typename Item, std::size_t Capacity>
typename InplaceVector<Item, Capacity>::Iterator InplaceVector<Item, Capacity>::EraseExchangingWithLast (
    const InplaceVector::Iterator &_iterator) noexcept requires std::is_nothrow_move_assignable_v<Item> &&
    std::is_nothrow_default_constructible_v<Item>
{
    assert (_iterator < End ());
    auto last = End () - 1u;

    if (_iterator != last)
    {
        *_iterator = std::move (*last);
    }
    else
    {
        // Reassign last item to default values, so it will not hold any real resources.
        *_iterator = Item ();
    }

    --count;
    return last;
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
} // namespace Emergence
