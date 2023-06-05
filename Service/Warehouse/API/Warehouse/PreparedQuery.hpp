#pragma once

#include <API/Common/Shortcuts.hpp>

#include <StandardLayout/Mapping.hpp>

#include <Visual/Graph.hpp>

namespace Emergence::Warehouse
{
class Registry;
} // namespace Emergence::Warehouse

/// \brief Internal macro with common operations for EMERGENCE_READ_ONLY_PREPARED_QUERY_OPERATIONS and
///        EMERGENCE_EDITABLE_PREPARED_QUERY_OPERATIONS.
#define EMERGENCE_PREPARED_QUERY_OPERATIONS_COMMON(Class)                                                              \
    Class (const Class &_other) noexcept;                                                                              \
                                                                                                                       \
    /* NOLINTNEXTLINE(bugprone-macro-parentheses): Types can not be enclosed. */                                       \
    Class (Class &&_other) noexcept;                                                                                   \
                                                                                                                       \
    /*! \invariant There is no active cursors for this query. */                                                       \
    ~Class () noexcept;                                                                                                \
                                                                                                                       \
    /*! \return Mapping for type, with which this query works. */                                                      \
    [[nodiscard]] Emergence::StandardLayout::Mapping GetTypeMapping () const noexcept;                                 \
                                                                                                                       \
    /*! \return Whether this query works with data from given registry. */                                             \
    [[nodiscard]] bool IsFromRegistry (const Warehouse::Registry &_registry) const noexcept;                           \
                                                                                                                       \
    /*! Assigning prepared queries looks counter-intuitive. */                                                         \
    EMERGENCE_DELETE_ASSIGNMENT (Class);                                                                               \
                                                                                                                       \
    /*! \brief Utility for Warehouse::Visualization, that allows implementation to add custom content to graphs. */    \
    void AddCustomVisualization (VisualGraph::Graph &_graph) const noexcept

/// \brief Adds common public methods for read only (fetch) prepared queries.
#define EMERGENCE_READONLY_PREPARED_QUERY_OPERATIONS(Class, Cursor, ...)                                               \
    EMERGENCE_PREPARED_QUERY_OPERATIONS_COMMON (Class);                                                                \
                                                                                                                       \
    /*!                                                                                                                \
     * \details Thread safe.                                                                                           \
     * \invariant There is no modification or insertion cursors for ::GetTypeMapping type in registry.                 \
     */                                                                                                                \
    [[nodiscard]] Cursor Execute (__VA_ARGS__) noexcept

/// \brief Adds common public methods for editable (insert and modify) prepared queries.
#define EMERGENCE_EDITABLE_PREPARED_QUERY_OPERATIONS(Class, Cursor, ...)                                               \
    EMERGENCE_PREPARED_QUERY_OPERATIONS_COMMON (Class);                                                                \
                                                                                                                       \
    /*! \invariant There is no other cursors for ::GetTypeMapping type in registry. */                                 \
    Cursor Execute (__VA_ARGS__) noexcept;                                                                             \
                                                                                                                       \
    /*!                                                                                                                \
     * \brief Allows to temporarily lift `no modify cursors` invariant for fetch queries                               \
     *        and `no fetch cursors` for edit queries.                                                                 \
     * \details By lifting these requirements user states that these invariants are true:                              \
     *          - There is no multi thread access to any query, that works with the same type, during this period.     \
     *          - Key fields of any fetch cursors are not modified by any modify cursors.                              \
     *          - New objects with the same type are not inserted.                                                     \
     *          - Modify cursor is not deleting objects with the same type.                                            \
     *          Breaking these invariants results in undefined behaviour.                                              \
     *                                                                                                                 \
     *          In most cases user should avoid entering unsafe fetch mode, but some rare tasks, that are usually      \
     *          connected to hierarchical access, can not be solved without this tricky approach.                      \
     */                                                                                                                \
    class UnsafeFetchAccessToken final                                                                                 \
    {                                                                                                                  \
    public:                                                                                                            \
        UnsafeFetchAccessToken (const UnsafeFetchAccessToken &_other) = delete;                                        \
                                                                                                                       \
        UnsafeFetchAccessToken (UnsafeFetchAccessToken &&_other) noexcept;                                             \
                                                                                                                       \
        ~UnsafeFetchAccessToken () noexcept;                                                                           \
                                                                                                                       \
        EMERGENCE_DELETE_ASSIGNMENT (UnsafeFetchAccessToken);                                                          \
                                                                                                                       \
    private:                                                                                                           \
        friend class Class;                                                                                            \
                                                                                                                       \
        UnsafeFetchAccessToken (void *_handle) noexcept;                                                               \
                                                                                                                       \
        EMERGENCE_BIND_IMPLEMENTATION_HANDLE ()                                                                       \
    };                                                                                                                 \
                                                                                                                       \
    /*! \see UnsafeFetchAccessToken */                                                                                 \
    UnsafeFetchAccessToken AllowUnsafeFetchAccess () noexcept
