#pragma once

/// \brief Adds common forward iterator operators, copy constructor, move constructor and destructor.
/// \details There is a lot of similar iterators in different services. Therefore it's better to hide their common
///          operations under macro to avoid unnecessary declaration and documentation duplication.
#define EMERGENCE_FORWARD_ITERATOR_OPERATIONS(IteratorClass, ItemClass)                                                \
    IteratorClass (const IteratorClass &_other) noexcept;                                                              \
                                                                                                                       \
    /* NOLINTNEXTLINE(bugprone-macro-parentheses): Types can not be enclosed. */                                       \
    IteratorClass (IteratorClass &&_other) noexcept;                                                                   \
                                                                                                                       \
    ~IteratorClass () noexcept;                                                                                        \
                                                                                                                       \
    /*!                                                                                                                \
     * \return Item, to which iterator points.                                                                         \
     * \invariant Inside valid bounds, but not in the ending.                                                          \
     */                                                                                                                \
    [[nodiscard]] ItemClass operator* () const noexcept; /* NOLINT(bugprone-macro-parentheses) */                      \
                                                                                                                       \
    /*!                                                                                                                \
     * \brief Move to next item.                                                                                       \
     * \invariant Inside valid bounds, but not in the ending.                                                          \
     */                                                                                                                \
    IteratorClass &operator++ () noexcept; /* NOLINT(bugprone-macro-parentheses): Types can not be enclosed. */        \
                                                                                                                       \
    /*!                                                                                                                \
     * \brief Move to next item.                                                                                       \
     * \return Unchanged instance of iterator.                                                                         \
     * \invariant Inside valid bounds, but not in the ending.                                                          \
     */                                                                                                                \
    IteratorClass operator++ (int) noexcept; /* NOLINT(bugprone-macro-parentheses): Types can not be enclosed. */      \
                                                                                                                       \
    bool operator== (const IteratorClass &_other) const noexcept;                                                      \
                                                                                                                       \
    bool operator!= (const IteratorClass &_other) const noexcept;                                                      \
                                                                                                                       \
    /* NOLINTNEXTLINE(bugprone-macro-parentheses): Types can not be enclosed. */                                       \
    IteratorClass &operator= (const IteratorClass &_other) noexcept;                                                   \
                                                                                                                       \
    /* NOLINTNEXTLINE(bugprone-macro-parentheses): Types can not be enclosed. */                                       \
    IteratorClass &operator= (IteratorClass &&_other) noexcept

/// \brief Extends EMERGENCE_FORWARD_ITERATOR_OPERATIONS with backward movement operations.
#define EMERGENCE_BIDIRECTIONAL_ITERATOR_OPERATIONS(IteratorClass, ItemClass)                                          \
    EMERGENCE_FORWARD_ITERATOR_OPERATIONS (IteratorClass, ItemClass);                                                  \
                                                                                                                       \
    /*!                                                                                                                \
     * \brief Move to previous item.                                                                                   \
     * \invariant Inside valid bounds, but not in the beginning.                                                       \
     */                                                                                                                \
    IteratorClass &operator-- () noexcept; /* NOLINT(bugprone-macro-parentheses): Types can not be enclosed. */        \
                                                                                                                       \
    /*!                                                                                                                \
     * \brief Move to previous item.                                                                                   \
     * \return Unchanged instance of iterator.                                                                         \
     * \invariant Inside valid bounds, but not in the beginning.                                                       \
     */                                                                                                                \
    IteratorClass operator-- (int) noexcept /* NOLINT(bugprone-macro-parentheses): Types can not be enclosed. */
