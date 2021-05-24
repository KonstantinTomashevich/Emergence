#pragma once

#include <concepts>

namespace Emergence::Handling
{
/// \brief Describes Handle requirements for its parameter Type.
template <typename Type>
concept Handleable = requires (Type _object)
{
    /// \brief Increments reference counter.
    { _object.RegisterReference () } noexcept;

    /// \brief Decrements reference counter.
    { _object.UnregisterReference () } noexcept;

    /// \return Is reference counter greater than zero?
    { _object.HasAnyReferences () } noexcept -> std::convertible_to <bool>;
};

/// \brief Strong reference to given object.
template <Handleable Type>
class Handle final
{
public:
    /// \brief Constructs handle to given instance.
    Handle (Type *_instance) noexcept;

    Handle (const Handle &_other) noexcept;

    Handle (Handle &&_other) noexcept;

    ~Handle () noexcept;

    Handle &operator = (const Handle &_other) noexcept;

    Handle &operator = (Handle &&_other) noexcept;

    /// \return ::instance
    Type *Get () const noexcept;

    /// \return ::instance
    Type *operator -> () const noexcept;

    /// \return Is ::instance not equal to `nullptr`?
    operator bool () const noexcept;

private:
    /// \brief Pointer to associated instance. Can be `nullptr`.
    Type *instance;
};

template <Handleable Type>
Handle <Type>::Handle (Type *_instance) noexcept
    : instance (_instance)
{
    if (instance)
    {
        instance->RegisterReference ();
    }
}

template <Handleable Type>
Handle <Type>::Handle (const Handle &_other) noexcept
    : Handle (_other.instance)
{
}

template <Handleable Type>
Handle <Type>::Handle (Handle &&_other) noexcept
    : instance (_other.instance)
{
    _other.instance = nullptr;
}

template <Handleable Type>
Handle <Type>::~Handle () noexcept
{
    if (instance)
    {
        instance->UnregisterReference ();
        if (!instance->HasAnyReferences ())
        {
            delete instance;
            instance = nullptr;
        }
    }
}

template <Handleable Type>
Handle <Type> &Handle <Type>::operator = (const Handle &_other) noexcept
{
    if (this != &_other)
    {
        this->~Handle ();
        new (this) Handle (_other);
    }

    return *this;
}

template <Handleable Type>
Handle <Type> &Handle <Type>::operator = (Handle &&_other) noexcept
{
    if (this != &_other)
    {
        this->~Handle ();
        new (this) Handle (std::move (_other));
    }

    return *this;
}

template <Handleable Type>
Type *Handle <Type>::Get () const noexcept
{
    return instance;
}

template <Handleable Type>
Type *Handle <Type>::operator -> () const noexcept
{
    return Get ();
}

template <Handleable Type>
Handle <Type>::operator bool () const noexcept
{
    return instance;
}
} // namespace Emergence::Handling