#pragma once

#include <cstdint>

#include <API/Common/ImplementationBinding.hpp>

#include <Memory/UniqueString.hpp>

namespace Emergence::Memory
{
/// \brief Integrates malloc-free mechanism with memory profiling routine.
/// \warning Heap is just wrapper for malloc-free, therefore, unlike other allocators,
///          it's object destruction does not lead to deallocation of records.
class Heap final
{
public:
    /// \param _groupId Memory allocation group id for profiling.
    explicit Heap (UniqueString _groupId) noexcept;

    /// Copying memory allocator looks counter-intuitive.
    Heap (const Heap &_other) = delete;

    Heap (Heap &&_other) noexcept;

    ~Heap () noexcept;

    /// \brief Acquires given amount of memory with default alignment.
    void *Acquire (size_t _bytes) noexcept;

    /// \brief Resizes given record pointer and copies it byte-to-byte to the new location if needed.
    /// \param _currentSize Record current size, required for the same reasons as `_bytes` in ::Release.
    void *Resize (void *_record, size_t _currentSize, size_t _newSize) noexcept;

    /// \brief Releases given record pointer.
    /// \param _bytes Record size, must be equal to allocated value, passed through ::Acquire or ::Resize.
    ///               We have no access to malloc internal data, therefore we rely on this parameter to
    ///               log information to memory profiler.
    void Release (void *_record, size_t _bytes) noexcept;

    /// Copy assigning memory allocator looks counter-intuitive.
    Heap &operator= (const Heap &_other) = delete;

    Heap &operator= (Heap &&_other) noexcept;

private:
    template <typename Type>
    friend class HeapSTD;

    /// \return Memory allocation group id, needed for HeadSTD.
    [[nodiscard]] UniqueString GetGroupId () const noexcept;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));
};

/// \brief Wraps Heap into C++ STD Allocator requirements.
template <typename Type>
class HeapSTD final
{
public:
    using value_type = Type;

    using is_always_equal = std::true_type;

    HeapSTD (UniqueString _groupId) noexcept : heap (_groupId)
    {
    }

    template <typename Other>
    HeapSTD ([[maybe_unused]] const HeapSTD<Other> &_other) noexcept : HeapSTD (_other.heap.GetGroupId ())
    {
    }

    HeapSTD (const HeapSTD &_other) noexcept : HeapSTD (_other.heap.GetGroupId ())
    {
    }

    HeapSTD (HeapSTD &&_other) noexcept = default;

    ~HeapSTD () = default;

    // NOLINTNEXTLINE(readability-identifier-naming): STD naming.
    Type *allocate (size_t _count) noexcept
    {
        return static_cast<Type *> (heap.Acquire (sizeof (Type) * _count));
    }

    // NOLINTNEXTLINE(readability-identifier-naming): STD naming.
    void deallocate (Type *_record, size_t _count) noexcept
    {
        heap.Release (_record, sizeof (Type) * _count);
    }

    HeapSTD &operator= (const HeapSTD &_other) = delete;

    HeapSTD &operator= (HeapSTD &&_other) = delete;

private:
    /// Wrappers of all types must be friends because of rebind constructor.
    template <typename Other>
    friend class HeapSTD;

    Heap heap;
};
} // namespace Emergence::Memory
