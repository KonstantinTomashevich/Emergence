#pragma once

#include <ContainerApi.hpp>

#include <functional>

#include <Memory/Heap.hpp>

namespace Emergence::Container
{
// TODO: Add tests for Any?

/// \brief Storage that can contain object of any type.
/// \details Technically, it is just a unique pointer with runtime type.
class ContainerApi Any final
{
public:
    /// \brief Constructs new empty container that allocates storage inside given allocation group.
    Any (Memory::Profiler::AllocationGroup _group) noexcept;

    Any (const Any &_other) = delete;

    Any (Any &&_other) noexcept;

    ~Any () noexcept;

    /// \return Pointer to storage.
    [[nodiscard]] const void *Get () const noexcept;

    /// \return Pointer to storage.
    void *Get () noexcept;

    /// \brief Allocates storage for given type and constructs it inplace.
    /// \warning If container already had some values, previous value is destructed.
    template <typename Type, typename... Arguments>
    Type &Emplace (Arguments &&..._constructionArguments) noexcept;

    /// \brief Destructs current value (if any) and returns to initial empty state.
    void Reset () noexcept;

    Any &operator= (const Any &_other) = delete;

    Any &operator= (Any &&_other) noexcept;

private:
    void *pointer = nullptr;
    std::size_t size = 0u;
    std::function<void (void *)> destructor;
    Memory::Heap heap;
};

template <typename Type, typename... Arguments>
Type &Any::Emplace (Arguments &&..._constructionArguments) noexcept
{
    Reset ();
    size = sizeof (Type);
    pointer = heap.Acquire (size, alignof (Type));
    new (pointer) Type (std::forward<Arguments> (_constructionArguments)...);

    destructor = [] (void *_pointer)
    {
        static_cast<Type *> (_pointer)->~Type ();
    };

    return *static_cast<Type *> (pointer);
}
} // namespace Emergence::Container
