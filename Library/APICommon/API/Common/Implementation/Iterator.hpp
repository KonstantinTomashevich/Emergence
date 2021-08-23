#pragma once

#include <SyntaxSugar/BlockCast.hpp>

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

/// \brief Binds all forward service iterator operations except dereference to implementation forward iterator.
#define EMERGENCE_BIND_FORWARD_ITERATOR_OPERATIONS_IMPLEMENTATION(ServiceClass, ImplementationClass)                   \
ServiceClass::ServiceClass (const ServiceClass &_other) noexcept                                                       \
{                                                                                                                      \
    new (&data) ImplementationClass (block_cast <ImplementationClass> (_other.data));                                  \
}                                                                                                                      \
                                                                                                                       \
ServiceClass::ServiceClass (ServiceClass &&_other) noexcept                                                            \
{                                                                                                                      \
    new (&data) ImplementationClass (std::move (block_cast <ImplementationClass> (_other.data)));                      \
}                                                                                                                      \
                                                                                                                       \
ServiceClass::~ServiceClass () noexcept                                                                                \
{                                                                                                                      \
    block_cast <ImplementationClass> (data).~ImplementationClass ();                                                   \
}                                                                                                                      \
                                                                                                                       \
ServiceClass &ServiceClass::operator ++ () noexcept                                                                    \
{                                                                                                                      \
    ++block_cast <ImplementationClass> (data);                                                                         \
    return *this;                                                                                                      \
}                                                                                                                      \
                                                                                                                       \
ServiceClass ServiceClass::operator ++ (int) noexcept                                                                  \
{                                                                                                                      \
    ImplementationClass previous = block_cast <ImplementationClass> (data)++;                                          \
    return ServiceClass (reinterpret_cast <decltype (data) *> (&previous));                                            \
}                                                                                                                      \
                                                                                                                       \
bool ServiceClass::operator == (const ServiceClass &_other) const noexcept                                             \
{                                                                                                                      \
    return block_cast <ImplementationClass> (data) == block_cast <ImplementationClass> (_other.data);                  \
}                                                                                                                      \
                                                                                                                       \
bool ServiceClass::operator != ( const ServiceClass &_other) const noexcept                                            \
{                                                                                                                      \
    return !(*this == _other);                                                                                         \
}                                                                                                                      \
                                                                                                                       \
ServiceClass &ServiceClass::operator = (const ServiceClass &_other) noexcept                                           \
{                                                                                                                      \
    if (this != &_other)                                                                                               \
    {                                                                                                                  \
        this->~ServiceClass ();                                                                                        \
        new (this) ServiceClass (_other);                                                                              \
    }                                                                                                                  \
                                                                                                                       \
    return *this;                                                                                                      \
}                                                                                                                      \
                                                                                                                       \
ServiceClass &ServiceClass::operator = (ServiceClass &&_other) noexcept                                                \
{                                                                                                                      \
    if (this != &_other)                                                                                               \
    {                                                                                                                  \
        this->~ServiceClass ();                                                                                        \
        new (this) ServiceClass (std::move (_other));                                                                  \
    }                                                                                                                  \
                                                                                                                       \
    return *this;                                                                                                      \
}                                                                                                                      \
                                                                                                                       \
ServiceClass::ServiceClass (const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept                                 \
{                                                                                                                      \
    new (&data) ImplementationClass (block_cast <ImplementationClass> (*_data));                                       \
}

/// \brief Binds all bidirectional service iterator operations
///        except dereference to implementation bidirectional iterator.
#define EMERGENCE_BIND_BIDIRECTIONAL_ITERATOR_OPERATIONS_IMPLEMENTATION(ServiceClass, ImplementationClass)             \
EMERGENCE_BIND_FORWARD_ITERATOR_OPERATIONS_IMPLEMENTATION (ServiceClass, ImplementationClass)                          \
                                                                                                                       \
ServiceClass &ServiceClass::operator -- () noexcept                                                                    \
{                                                                                                                      \
    --block_cast <ImplementationClass> (data);                                                                         \
    return *this;                                                                                                      \
}                                                                                                                      \
                                                                                                                       \
ServiceClass ServiceClass::operator -- (int) noexcept                                                                  \
{                                                                                                                      \
    ImplementationClass previous = block_cast <ImplementationClass> (data)--;                                          \
    return ServiceClass (reinterpret_cast <decltype (data) *> (&previous));                                            \
}
