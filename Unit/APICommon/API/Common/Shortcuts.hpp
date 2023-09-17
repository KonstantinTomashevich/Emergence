#pragma once

/// \brief Shortcut, that deletes both copy and move assignment operations for given class.
#define EMERGENCE_DELETE_ASSIGNMENT(Class)                                                                             \
    Class &operator= (const Class &_other) = delete;                                                                   \
    /* NOLINTNEXTLINE(bugprone-macro-parentheses): Types can not be enclosed. */                                       \
    Class &operator= (Class &&_other) = delete

/// \brief Shortcut for declaration of classes that can not be copied, moved or assigned
///        and have no-argument constructor and custom destructor.
#define EMERGENCE_STATIONARY_DATA_TYPE(Class)                                                                          \
    Class () noexcept;                                                                                                 \
                                                                                                                       \
    Class (const Class &_other) = delete;                                                                              \
                                                                                                                       \
    /* NOLINTNEXTLINE(bugprone-macro-parentheses): Types can not be enclosed. */                                       \
    Class (Class &&_other) = delete;                                                                                   \
                                                                                                                       \
    ~Class () noexcept;                                                                                                \
                                                                                                                       \
    EMERGENCE_DELETE_ASSIGNMENT (Class);
