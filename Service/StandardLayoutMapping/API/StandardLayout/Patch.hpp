#pragma once

#include <cstdint>

#include <API/Common/ImplementationBinding.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::StandardLayout
{
/// \brief Contains set of changes that transform object from one state to another.
///
/// \details PatchBuilder should be used to construct patches.
///          Patches should be implemented as handles to the real data to make copying as lightweight as possible.
class Patch final
{
public:
    /// \brief Describes change info that can be retrieved during iteration using Iterator.
    struct ChangeInfo
    {
        FieldId field;
        const void *newValue = nullptr;
    };

    /// \brief Allows iteration over Patch transformation changelist.
    class Iterator final
    {
    public:
        EMERGENCE_BIDIRECTIONAL_ITERATOR_OPERATIONS (Iterator, ChangeInfo);

    private:
        /// Patch constructs iterators.
        friend class Patch;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t));

        explicit Iterator (const std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept;
    };

    /// \brief Constructs invalid patch. Needed only to simplify registration procedure and must be reassigned.
    Patch () noexcept;

    Patch (const Patch &_other) noexcept;

    Patch (Patch &&_other) noexcept;

    ~Patch () noexcept;

    /// \return Type of objects to which this patch can be applied.
    [[nodiscard]] Mapping GetTypeMapping () const noexcept;

    /// \brief Applies transformation, defined by patch, to given object.
    /// \details If object state is different from the initial state for which patch was created,
    ///          transformation result would be different too.
    /// \invariant Object belongs to ::GetTypeMapping type.
    void Apply (void *_object) const noexcept;

    /// \return Count of changes in transformation.
    [[nodiscard]] std::size_t GetChangeCount () const noexcept;

    /// \return Iterator, that points to the beginning of transformation changelist.
    [[nodiscard]] Iterator Begin () const noexcept;

    /// \return Iterator, that points to thee ending of transformation changelist.
    [[nodiscard]] Iterator End () const noexcept;

    /// \return Whether this and given patch handles point to the same implementation.
    /// \details It is implemented as separate method instead of `==`, because this check returns `false`
    ///          when patches are technically equal, but were built separately.
    [[nodiscard]] bool IsHandleEqual (const Patch &_other) const noexcept;

    /// \brief Combines changes from this and given patch resolving all conflicts in favor of given patch.
    /// \invariant Both patches were build for one mapping!
    /// \warning Not commutative!
    Patch operator+ (const Patch &_other) const noexcept;

    /// \brief Creates new patch that contains changes which are listed in this patch, but not in given patch.
    /// \details `x = y - z => y = z + x`. Keep in mind that `+` is not commutative!
    /// \invariant Both patches were build for one mapping!
    Patch operator- (const Patch &_other) const noexcept;

    /// \return Whether this instance points to a valid patch.
    /// \return Is field ::handle valid?
    [[nodiscard]] bool IsHandleValid () const noexcept;

    /// \return ::IsHandleValid ()
    explicit operator bool () const noexcept;

    Patch &operator= (const Patch &_other) noexcept;

    Patch &operator= (Patch &&_other) noexcept;

private:
    friend class PatchBuilder;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t));

    /// \brief Copies implementation-specific values from given pointer.
    explicit Patch (const std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept;

    /// \brief Moves implementation-specific values from given pointer.
    explicit Patch (std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept;
};

/// \brief Wraps Patch::Begin for foreach sentences.
Patch::Iterator begin (const Patch &_patch) noexcept;

/// \brief Wraps Patch::End for foreach sentences.
Patch::Iterator end (const Patch &_patch) noexcept;
} // namespace Emergence::StandardLayout
