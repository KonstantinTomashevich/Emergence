#pragma once

#include <SyntaxSugar/BlockCast.hpp>

/// \brief Binds all read cursor operations to implementation read cursor.
/// \see EMERGENCE_READ_CURSOR_OPERATIONS
#define EMERGENCE_BIND_READ_CURSOR_OPERATIONS_IMPLEMENTATION(ServiceClass, ImplementationClass)                        \
    ServiceClass::ServiceClass (const ServiceClass &_other) noexcept                                                   \
    {                                                                                                                  \
        new (&data) ImplementationClass (block_cast<ImplementationClass> (_other.data));                               \
    }                                                                                                                  \
                                                                                                                       \
    /* NOLINTNEXTLINE(bugprone-macro-parentheses): Types can not be enclosed. */                                       \
    ServiceClass::ServiceClass (ServiceClass &&_other) noexcept                                                        \
    {                                                                                                                  \
        new (&data) ImplementationClass (std::move (block_cast<ImplementationClass> (_other.data)));                   \
    }                                                                                                                  \
                                                                                                                       \
    ServiceClass::~ServiceClass () noexcept                                                                            \
    {                                                                                                                  \
        block_cast<ImplementationClass> (data).~ImplementationClass ();                                                \
    }                                                                                                                  \
                                                                                                                       \
    const void *ServiceClass::operator* () const noexcept                                                              \
    {                                                                                                                  \
        return *block_cast<ImplementationClass> (data);                                                                \
    }                                                                                                                  \
                                                                                                                       \
    /* NOLINTNEXTLINE(bugprone-macro-parentheses): Types can not be enclosed. */                                       \
    ServiceClass &ServiceClass::operator++ () noexcept                                                                 \
    {                                                                                                                  \
        ++block_cast<ImplementationClass> (data);                                                                      \
        return *this;                                                                                                  \
    }                                                                                                                  \
                                                                                                                       \
    ServiceClass::ServiceClass (std::array<uint8_t, DATA_MAX_SIZE> &_data) noexcept                                    \
    {                                                                                                                  \
        new (&data) ImplementationClass (std::move (block_cast<ImplementationClass> (_data)));                         \
    }

/// \brief Binds all edit cursor operations to implementation read cursor.
/// \see EMERGENCE_EDIT_CURSOR_OPERATIONS
#define EMERGENCE_BIND_EDIT_CURSOR_OPERATIONS_IMPLEMENTATION(ServiceClass, ImplementationClass)                        \
    /* NOLINTNEXTLINE(bugprone-macro-parentheses): Types can not be enclosed. */                                       \
    ServiceClass::ServiceClass (ServiceClass &&_other) noexcept                                                        \
    {                                                                                                                  \
        new (&data) ImplementationClass (std::move (block_cast<ImplementationClass> (_other.data)));                   \
    }                                                                                                                  \
                                                                                                                       \
    ServiceClass::~ServiceClass () noexcept                                                                            \
    {                                                                                                                  \
        block_cast<ImplementationClass> (data).~ImplementationClass ();                                                \
    }                                                                                                                  \
                                                                                                                       \
    void *ServiceClass::operator* () noexcept                                                                          \
    {                                                                                                                  \
        return *block_cast<ImplementationClass> (data);                                                                \
    }                                                                                                                  \
                                                                                                                       \
    /* NOLINTNEXTLINE(bugprone-macro-parentheses): Types can not be enclosed. */                                       \
    ServiceClass &ServiceClass::operator~ () noexcept                                                                  \
    {                                                                                                                  \
        ~block_cast<ImplementationClass> (data);                                                                       \
        return *this;                                                                                                  \
    }                                                                                                                  \
                                                                                                                       \
    /* NOLINTNEXTLINE(bugprone-macro-parentheses): Types can not be enclosed. */                                       \
    ServiceClass &ServiceClass::operator++ () noexcept                                                                 \
    {                                                                                                                  \
        ++block_cast<ImplementationClass> (data);                                                                      \
        return *this;                                                                                                  \
    }                                                                                                                  \
                                                                                                                       \
    ServiceClass::ServiceClass (std::array<uint8_t, DATA_MAX_SIZE> &_data) noexcept                                    \
    {                                                                                                                  \
        new (&data) ImplementationClass (std::move (block_cast<ImplementationClass> (_data)));                         \
    }
