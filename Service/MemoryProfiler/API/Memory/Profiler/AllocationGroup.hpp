#pragma once

#include <cstdint>

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Iterator.hpp>
#include <API/Common/Shortcuts.hpp>

#include <Memory/UniqueString.hpp>

namespace Emergence::Memory::Profiler
{
/// \brief Logical group for tracking one or more logically connected allocators. Might be a subgroup of another group.
/// \details Grouping makes it easy to distinguish most aggressive memory consumers in a program.
///          Group hierarchy provides easy way to go from "big picture" view to details and vice versa.
///          For example, it allows to descend from statement "Records eat much more memory than they should"
///          to "There is too much indices for record types X and Y" without any additional debugging or code edition.
class AllocationGroup final
{
public:
    /// \brief RAII object for AllocationGroup stack management.
    /// \see AllocationGroup::Top
    class PlacedOnStack final
    {
    public:
        PlacedOnStack (const PlacedOnStack &_other) = delete;

        PlacedOnStack (PlacedOnStack &&_other) = delete;

        // NOLINTNEXTLINE(performance-trivially-destructible): Trivially destructible only for None implementation.
        ~PlacedOnStack ();

        EMERGENCE_DELETE_ASSIGNMENT (PlacedOnStack);

    private:
        /// AllocationGroup constructs stack placeholders.
        friend class AllocationGroup;

        explicit PlacedOnStack (void *_groupHandle) noexcept;
    };

    /// \brief Provides iteration over AllocationGroup children.
    class Iterator final
    {
    public:
        EMERGENCE_FORWARD_ITERATOR_OPERATIONS (Iterator, AllocationGroup);

    private:
        /// AllocationGroup constructs iterators.
        friend class AllocationGroup;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t));

        explicit Iterator (const std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept;
    };

    /// \return Root group of the AllocationGroup tree. Always exists.
    /// \details Do not explicitly make ::Root parent of new allocation groups unless it is truly necessary,
    ///          because it blocks module users from incorporating module groups into bigger application tree through
    ///          allocation group stack.
    static AllocationGroup Root () noexcept;

    /// \return Top group on thread local group stack.
    /// \details Group stack makes it easy to incorporate allocation group structure of local modules into
    ///          allocation group tree of entire application by avoiding explicit specification of group
    ///          parent, which makes these groups children of the ::Top group. For example we may incorporate
    ///          several different containers, like RecordCollection, into application-level world, like Registry.
    ///          Stack is thread local, because it's impossible to imagine how shared stack should work
    ///          and for which tasks this approach is really needed.
    static AllocationGroup Top () noexcept;

    /// \brief Creates empty placeholder allocation group, that has zero logic.
    /// \details Thread safe.
    AllocationGroup () noexcept;

    /// \brief Creates new group with given id, that is child of ::Top.
    /// \details If such group is already exists, nothing is overwritten.
    ///          Thread safe. Invalidates iterators of parent group.
    explicit AllocationGroup (UniqueString _id) noexcept;

    /// \brief Constructs allocation group as child of other given allocation group.
    /// \details If such group is already exists, nothing is overwritten.
    ///          Thread safe. Invalidates iterators of parent group.
    AllocationGroup (const AllocationGroup &_parent, UniqueString _id) noexcept;

    /// \details Thread safe.
    AllocationGroup (const AllocationGroup &_other) noexcept;

    /// \details Thread safe.
    AllocationGroup (AllocationGroup &&_other) noexcept;

    /// \details Thread safe.
    ~AllocationGroup () noexcept; // NOLINT(performance-trivially-destructible): Implementation-specific.

    /// \brief Places this group on top of thread local stack.
    /// \return RAII object for stack push/pop operations.
    /// \see ::Top
    /// \invariant Cannot be called by placeholder group.
    [[nodiscard]] PlacedOnStack PlaceOnTop () const noexcept;

    /// \brief Record allocation of given amount of bytes.
    /// \details Allocated bytes are reserved for this group, but not yet used by any object.
    ///          Thread safe.
    void Allocate (std::size_t _bytesCount) noexcept;

    /// \brief Record acquisition of given amount of bytes for usage.
    /// \invariant ::GetReserved >= _bytesCount
    /// \details Thread safe.
    void Acquire (std::size_t _bytesCount) noexcept;

    /// \brief Record that given amount of bytes is no longer used by any object.
    /// \invariant ::GetAcquired >= _bytesCount
    /// \details Thread safe.
    void Release (std::size_t _bytesCount) noexcept;

    /// \brief Record that given amount of bytes is no longer owned by this group.
    /// \invariant ::GetReserved >= _bytesCount
    /// \details Thread safe.
    void Free (std::size_t _bytesCount) noexcept;

    /// \return Allocation group, that contains this group as subgroup.
    /// \details Parent of ::Root is empty placeholder group, which in turn is the parent of itself.
    [[nodiscard]] AllocationGroup Parent () const noexcept;

    [[nodiscard]] Iterator BeginChildren () const noexcept;

    [[nodiscard]] Iterator EndChildren () const noexcept;

    /// \return Id of this group, unique among children of its ::Parent.
    [[nodiscard]] UniqueString GetId () const noexcept;

    /// \return Amount of bytes, that are used by any object.
    [[nodiscard]] std::size_t GetAcquired () const noexcept;

    /// \return Amount of bytes, that are not used by any object, but reserved for usage in future.
    [[nodiscard]] std::size_t GetReserved () const noexcept;

    /// \return Total amount of bytes, that are owned by allocation group.
    [[nodiscard]] std::size_t GetTotal () const noexcept;

    /// \details Guaranteed to be unique, but does not persist through program executions.
    [[nodiscard]] std::uintptr_t Hash () const noexcept;

    AllocationGroup &operator= (const AllocationGroup &_other) noexcept;

    AllocationGroup &operator= (AllocationGroup &&_other) noexcept;

    bool operator== (const AllocationGroup &_other) const noexcept;

    bool operator!= (const AllocationGroup &_other) const noexcept;

private:
    /// \brief Allows construction from handle for implementation internal purposes.
    friend class ImplementationUtils;

    EMERGENCE_BIND_IMPLEMENTATION_HANDLE ()

    explicit AllocationGroup (void *_handle) noexcept;
};
} // namespace Emergence::Memory::Profiler

namespace std
{
template <>
struct hash<Emergence::Memory::Profiler::AllocationGroup>
{
    std::size_t operator() (const Emergence::Memory::Profiler::AllocationGroup &_group) const noexcept
    {
        return static_cast<std::size_t> (_group.Hash ());
    }
};
} // namespace std
