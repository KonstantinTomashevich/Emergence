#pragma once

/// \brief Shortcut, that deletes both copy and move assignment operations for given class.
#define EMERGENCE_DELETE_ASSIGNMENT(Class)                                                                             \
    Class &operator= (const Class &_other) = delete;                                                                   \
    /* NOLINTNEXTLINE(bugprone-macro-parentheses): Types can not be enclosed. */                                       \
    Class &operator= (Class &&_other) = delete
