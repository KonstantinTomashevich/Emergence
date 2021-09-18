#pragma once

#include <API/Common/Shortcuts.hpp>

/// \brief Adds common read cursor operators, copy constructor, move constructor
///        and destructor, deletes assignment operators.
/// \details There is a lot of similar read cursors in different services. Therefore it's better to hide their common
///          operations under macro to avoid unnecessary declaration and documentation duplication.
#define EMERGENCE_READ_CURSOR_OPERATIONS(Class)                                                                        \
    Class (const Class &_other) noexcept;                                                                              \
                                                                                                                       \
    Class (Class &&_other) noexcept;                                                                                   \
                                                                                                                       \
    ~Class () noexcept;                                                                                                \
                                                                                                                       \
    /*! \return Pointer to current item or nullptr if cursor points to ending. */                                      \
    const void *operator* () const noexcept;                                                                           \
                                                                                                                       \
    /*!                                                                                                                \
     * \brief Moves cursor to next item.                                                                               \
     * \invariant Cursor should not point to sequence ending.                                                          \
     */                                                                                                                \
    Class &operator++ () noexcept;                                                                                     \
                                                                                                                       \
    /*! Assigning cursors looks counter intuitive. */                                                                  \
    EMERGENCE_DELETE_ASSIGNMENT (Class)

/// \brief Adds common edit cursor operators, move constructor and destructor,
///        deletes copy constructor and assignment operators.
/// \details There is a lot of similar edit cursors in different services. Therefore it's better to hide their common
///          operations under macro to avoid unnecessary declaration and documentation duplication.
#define EMERGENCE_EDIT_CURSOR_OPERATIONS(Class)                                                                        \
    /*!                                                                                                                \
     * Edit cursors can not be copied, because not more than one edit                                                  \
     * cursor can exist inside one Collection at any moment of time.                                                   \
     */                                                                                                                \
    Class (const Class &_other) = delete;                                                                              \
                                                                                                                       \
    Class (Class &&_other) noexcept;                                                                                   \
                                                                                                                       \
    ~Class () noexcept;                                                                                                \
                                                                                                                       \
    /*! \return Pointer to current item or nullptr if cursor points to ending. */                                      \
    void *operator* () noexcept;                                                                                       \
                                                                                                                       \
    /*!                                                                                                                \
     * \brief Deletes current item from collection and moves to next item.                                             \
     *                                                                                                                 \
     * \invariant Cursor should not point to ending.                                                                   \
     *                                                                                                                 \
     * \warning Item type is unknown during compile time, therefore appropriate                                        \
     *          destructor should be called before item deletion.                                                      \
     */                                                                                                                \
    Class &operator~ () noexcept;                                                                                      \
                                                                                                                       \
    /*!                                                                                                                \
     * \brief Checks current item for changes. Then moves cursor to next item.                                         \
     * \invariant Cursor should not point to ending.                                                                   \
     */                                                                                                                \
    Class &operator++ () noexcept;                                                                                     \
                                                                                                                       \
    /*! Assigning cursors looks counter intuitive. */                                                                  \
    EMERGENCE_DELETE_ASSIGNMENT (Class)
