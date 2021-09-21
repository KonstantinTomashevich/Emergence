#pragma once

#include <API/Common/Shortcuts.hpp>

#include <StandardLayout/Mapping.hpp>

#include <Visual/Graph.hpp>

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
    Emergence::StandardLayout::Mapping GetTypeMapping () const noexcept;                                               \
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
    Cursor Execute (__VA_ARGS__) noexcept;

/// \brief Adds common public methods for editable (insert and modify) prepared queries.
#define EMERGENCE_EDITABLE_PREPARED_QUERY_OPERATIONS(Class, Cursor, ...)                                               \
    EMERGENCE_PREPARED_QUERY_OPERATIONS_COMMON (Class);                                                                \
                                                                                                                       \
    /*! \invariant There is no other cursors for ::GetTypeMapping type in registry. */                                 \
    Cursor Execute (__VA_ARGS__) noexcept
