#pragma once

#include <concepts>

namespace Emergence::Handling
{
/// \brief Describes Handle requirements for its parameter Type.
template <typename Type>
concept Handleable = requires (Type _object) {
                         /// \brief Increments reference counter.
                         {
                             _object.RegisterReference ()
                         } noexcept;

                         /// \brief Decrements reference counter.
                         {
                             _object.UnregisterReference ()
                         } noexcept;

                         /// \return Reference counter value.
                         {
                             _object.GetReferenceCount ()
                             } noexcept -> std::convertible_to<std::uintptr_t>;
                     };

/// \brief When there is no references to object, callback will be called instead of object deletion.
/// \details In some cases, Handle is used to provide reference counting, not to fully control object lifetime.
template <typename Type>
concept HasLastReferenceUnregisteredCallback = requires (Type _object) {
                                                   {
                                                       _object.LastReferenceUnregistered ()
                                                   } noexcept;
                                               };

/// \brief Strong reference to given object.
/// \details ::Type must be Handleable, but requirement is expressed using static asserts in methods that directly use
///          Handleable methods. Static asserts are used instead of usual requirements because it allows to use Handle
///          with incomplete types.
template <typename Type>
class Handle final
{
public:
    /// \brief Constructs handle to given instance.
    Handle (Type *_instance) noexcept;

    Handle (const Handle &_other) noexcept;

    Handle (Handle &&_other) noexcept;

    ~Handle () noexcept;

    /// \return ::instance
    [[nodiscard]] Type *Get () const noexcept;

    /// \return ::instance
    Type *operator->() const noexcept;

    Handle &operator= (const Handle &_other) noexcept;

    Handle &operator= (Handle &&_other) noexcept;

    bool operator== (const Handle &_other) const noexcept = default;

    bool operator!= (const Handle &_other) const noexcept = default;

    /// \return Return true if ::instance is not nullptr.
    explicit operator bool () const noexcept;

private:
    /// \brief Pointer to associated instance. Can be `nullptr`.
    Type *instance;
};

template <typename Type>
Handle<Type>::Handle (Type *_instance) noexcept
    : instance (_instance)
{
    static_assert (Handleable<Type>);
    if (instance)
    {
        instance->RegisterReference ();
    }
}

template <typename Type>
Handle<Type>::Handle (const Handle &_other) noexcept
    : Handle (_other.instance)
{
}

template <typename Type>
Handle<Type>::Handle (Handle &&_other) noexcept
    : instance (_other.instance)
{
    _other.instance = nullptr;
}

template <typename Type>
Handle<Type>::~Handle () noexcept
{
    static_assert (Handleable<Type>);
    if (instance)
    {
        instance->UnregisterReference ();
        if (instance->GetReferenceCount () == 0u)
        {
            if constexpr (HasLastReferenceUnregisteredCallback<Type>)
            {
                if (instance)
                {
                    instance->LastReferenceUnregistered ();
                }
            }
            else
            {
                delete instance;
            }

            instance = nullptr;
        }
    }
}

template <typename Type>
Type *Handle<Type>::Get () const noexcept
{
    return instance;
}

template <typename Type>
Type *Handle<Type>::operator->() const noexcept
{
    return Get ();
}

template <typename Type>
// NOLINTNEXTLINE(bugprone-unhandled-self-assignment): Self assignment is handled, looks like a clang tidy bug.
Handle<Type> &Handle<Type>::operator= (const Handle &_other) noexcept
{
    if (this != &_other)
    {
        this->~Handle ();
        new (this) Handle (_other);
    }

    return *this;
}

template <typename Type>
Handle<Type> &Handle<Type>::operator= (Handle &&_other) noexcept
{
    if (this != &_other)
    {
        this->~Handle ();
        new (this) Handle (std::move (_other));
    }

    return *this;
}

template <typename Type>
Handle<Type>::operator bool () const noexcept
{
    return instance;
}
} // namespace Emergence::Handling
