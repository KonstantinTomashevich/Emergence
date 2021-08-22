#pragma once

/// \brief Implements all operations except dereference for forward iterator
///        that uses other forward iterator as backend.
/// \details It's quite common to wrap other iterator in order to override dereference with custom operator.
#define EMERGENCE_IMPLEMENT_FORWARD_ITERATOR_OPERATIONS_AS_WRAPPER(Class, baseIteratorField)                           \
Class::Class (const Class &_other) noexcept = default;                                                                 \
                                                                                                                       \
Class::Class (Class &&_other) noexcept = default;                                                                      \
                                                                                                                       \
Class::~Class () noexcept = default;                                                                                   \
                                                                                                                       \
Class &Class::operator ++ () noexcept                                                                                  \
{                                                                                                                      \
    ++baseIteratorField;                                                                                               \
    return *this;                                                                                                      \
}                                                                                                                      \
                                                                                                                       \
Class Class::operator ++ (int) noexcept                                                                                \
{                                                                                                                      \
    auto oldIterator = baseIteratorField;                                                                              \
    ++baseIteratorField;                                                                                               \
    return Class (oldIterator);                                                                                        \
}                                                                                                                      \
                                                                                                                       \
bool Class::operator == (const Class &_other) const noexcept                                                           \
{                                                                                                                      \
    return baseIteratorField == _other.baseIteratorField;                                                              \
}                                                                                                                      \
                                                                                                                       \
bool Class::operator != (const Class &_other) const noexcept                                                           \
{                                                                                                                      \
    return !(*this == _other);                                                                                         \
}                                                                                                                      \
                                                                                                                       \
Class &Class::operator = (const Class &_other) noexcept = default;                                                     \
                                                                                                                       \
Class &Class::operator = (Class &&_other) noexcept = default;                                                          \
                                                                                                                       \
Class::Class (decltype (baseIteratorField) _baseIterator) noexcept                                                     \
    : baseIteratorField (std::move (_baseIterator))                                                                    \
{                                                                                                                      \
}

/// \brief Implements all operations except dereference for bidirectional iterator
///        that uses other bidirectional iterator as backend.
/// \details It's quite common to wrap other iterator in order to override dereference with custom operator.
#define EMERGENCE_IMPLEMENT_BIDIRECTIONAL_ITERATOR_OPERATIONS_AS_WRAPPER(Class, baseIteratorField)                     \
EMERGENCE_IMPLEMENT_FORWARD_ITERATOR_OPERATIONS_AS_WRAPPER (Class, baseIteratorField)                                  \
                                                                                                                       \
Class &Class::operator -- () noexcept                                                                                  \
{                                                                                                                      \
    --baseIteratorField;                                                                                               \
    return *this;                                                                                                      \
}                                                                                                                      \
                                                                                                                       \
Class Class::operator -- (int) noexcept                                                                                \
{                                                                                                                      \
    auto oldIterator = baseIteratorField;                                                                              \
    --baseIteratorField;                                                                                               \
    return Class (oldIterator);                                                                                        \
}

// TODO: Add macros for iterator implementation binding.