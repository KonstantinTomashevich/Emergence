#pragma once

#include <MemoryApi.hpp>

#include <cstdint>

#include <API/Common/ImplementationBinding.hpp>

#include <Memory/Profiler/AllocationGroup.hpp>

namespace Emergence::Memory
{
/// \brief Integrates malloc-free mechanism with memory profiling routine.
/// \warning Heap is just wrapper for malloc-free, therefore, unlike other allocators,
///          it's object destruction does not lead to deallocation of records. But it
///          also means that this allocator is thread safe.
class MemoryApi Heap final
{
public:
    explicit Heap (Profiler::AllocationGroup _group) noexcept;

    /// Copying memory allocator looks counter-intuitive.
    Heap (const Heap &_other) = delete;

    Heap (Heap &&_other) noexcept;

    ~Heap () noexcept;

    /// \brief Acquires given amount of memory with given alignment.
    void *Acquire (std::size_t _bytes, std::size_t _alignment) noexcept;

    /// \brief Resizes given record pointer and copies it byte-to-byte to the new location if needed.
    /// \param _currentSize Record current size, required for the same reasons as `_bytes` in ::Release.
    void *Resize (void *_record, std::size_t _alignment, std::size_t _currentSize, std::size_t _newSize) noexcept;

    /// \brief Releases given record pointer.
    /// \param _bytes Record size, must be equal to allocated value, passed through ::Acquire or ::Resize.
    ///               We have no access to malloc internal data, therefore we rely on this parameter to
    ///               log information to memory profiler.
    void Release (void *_record, std::size_t _bytes) noexcept;

    /// \return Allocation group to which this allocator belongs.
    /// \warning Group will report zero memory usage if it is a placeholder or
    ///          if executable is linked to no-profile implementation.
    [[nodiscard]] const Profiler::AllocationGroup &GetAllocationGroup () const noexcept;

    /// \brief Comparison needed for some STL implementations.
    bool operator== (const Heap &_other) const noexcept;

    /// \brief Comparison needed for some STL implementations.
    bool operator!= (const Heap &_other) const noexcept;

    /// Copy assigning memory allocator looks counter-intuitive.
    Heap &operator= (const Heap &_other) = delete;

    Heap &operator= (Heap &&_other) noexcept;

private:
    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t));
};

// Only MSVC STL makes it possible to use undefined template for incorrect
// allocation detection. Other STLs require heap default constructor to
// be functional, therefore we need default "Undocumented" group.
#if defined(_MSVC_STL_VERSION)
template <typename Type>
struct DefaultAllocationGroup;
#else
template <typename Type>
struct DefaultAllocationGroup
{
    static Profiler::AllocationGroup Get () noexcept
    {
        return Profiler::AllocationGroup {Profiler::AllocationGroup::Top (), UniqueString {"Undocumented"}};
    }
};
#endif

/// \brief Wraps Heap into C++ STD Allocator requirements.
/// \details Cannot be final as some STL implementations inherit from allocator.
template <typename Type>
class HeapSTD
{
public:
    using value_type = Type;

    using is_always_equal = std::true_type;

    HeapSTD () noexcept
        : heap (DefaultAllocationGroup<Type>::Get ())
    {
    }

    /// \details Intentionally implicit to make container initialization easier.
    HeapSTD (const Profiler::AllocationGroup &_group) noexcept
        : heap (_group)
    {
    }

    /// \details Intentionally implicit to make container initialization easier.
    HeapSTD (const Heap &_heap) noexcept
        : heap (_heap.GetAllocationGroup ())
    {
    }

    template <typename Other>
    HeapSTD ([[maybe_unused]] const HeapSTD<Other> &_other) noexcept
        : HeapSTD (_other.GetAllocationGroup ())
    {
    }

    HeapSTD (const HeapSTD &_other) noexcept
        : HeapSTD (_other.GetAllocationGroup ())
    {
    }

    HeapSTD (HeapSTD &&_other) noexcept = default;

    ~HeapSTD () = default;

    // NOLINTNEXTLINE(readability-identifier-naming): STD naming.
    Type *allocate (std::size_t _count) noexcept
    {
        // NOLINTNEXTLINE(bugprone-sizeof-expression): Type might be pointer.
        return static_cast<Type *> (heap.Acquire (sizeof (Type) * _count, alignof (Type)));
    }

    // NOLINTNEXTLINE(readability-identifier-naming): STD naming.
    void deallocate (Type *_record, std::size_t _count) noexcept
    {
        // NOLINTNEXTLINE(bugprone-sizeof-expression): Type might be pointer.
        heap.Release (_record, sizeof (Type) * _count);
    }

    [[nodiscard]] const Profiler::AllocationGroup &GetAllocationGroup () const noexcept
    {
        return heap.GetAllocationGroup ();
    }

    /// \brief Comparison needed for some STL implementations.
    bool operator== (const HeapSTD &_other) const noexcept = default;

    /// \brief Comparison needed for some STL implementations.
    bool operator!= (const HeapSTD &_other) const noexcept = default;

    HeapSTD &operator= (const HeapSTD &_other) = delete;

    HeapSTD &operator= (HeapSTD &&_other) = delete;

private:
    Heap heap;
};
} // namespace Emergence::Memory

/// \brief Shortcut for DefaultAllocationGroup specialization for given type.
#define EMERGENCE_MEMORY_DEFAULT_ALLOCATION_GROUP(Api, Type)                                                           \
    namespace Emergence::Memory                                                                                        \
    {                                                                                                                  \
    template <>                                                                                                        \
    struct Api DefaultAllocationGroup<Type>                                                                            \
    {                                                                                                                  \
        static Profiler::AllocationGroup Get () noexcept;                                                              \
    };                                                                                                                 \
    }
