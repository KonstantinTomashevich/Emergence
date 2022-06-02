#pragma once

#include <API/Common/Cursor.hpp>

#include <Celerity/Event/EventTrigger.hpp>

/// \brief Root macro for declaring Celerity query wrappers.
/// \warning Query type specific methods, like GetKeyField, are omitted to simplify wrapping. These methods are intended
///          to be used when query purpose is not fully known, but Celerity end user usually knows it, because queries
///          are created by tasks, therefore this methods are not needed.
/// \details Macros are used instead of template to avoid increasing compilation time: these queries are likely to be
///          used in almost every task, therefore number of template instantiations per translation unit could be huge.
/// \see EMERGENCE_CELERITY_MODIFY_QUERY_WRAPPER_DECLARATION
/// \see EMERGENCE_CELERITY_EDIT_QUERY_WRAPPER_DECLARATION
/// \see EMERGENCE_CELERITY_REMOVE_QUERY_WRAPPER_DECLARATION
#define EMERGENCE_CELERITY_WRITE_QUERY_WRAPPER_DECLARATION(QueryClass, BaseClass, EditableReadModifier,                \
                                                           ConstReadModifier, DeleteModifier, ...)                     \
    /** \brief Wrapper for BaseClass, that integrates insertion with Celerity events. */                               \
    class QueryClass final                                                                                             \
    {                                                                                                                  \
    public:                                                                                                            \
        class Cursor final                                                                                             \
        {                                                                                                              \
        public:                                                                                                        \
            /*!                                                                                                        \
             * Edit cursors can not be copied, because not more than one edit                                          \
             * cursor can exist inside one Collection at any moment of time.                                           \
             */                                                                                                        \
            Cursor (const Cursor &_other) = delete;                                                                    \
                                                                                                                       \
            /* NOLINTNEXTLINE(bugprone-macro-parentheses): Types can not be enclosed. */                               \
            Cursor (Cursor &&_other) noexcept;                                                                         \
                                                                                                                       \
            ~Cursor () noexcept;                                                                                       \
                                                                                                                       \
            /*! \return Pointer to current item or nullptr if cursor points to ending. */                              \
            [[nodiscard]] void *operator* () noexcept EditableReadModifier;                                            \
                                                                                                                       \
            /*! \return Pointer to current item or nullptr if cursor points to ending. */                              \
            [[nodiscard]] const void *ReadConst () noexcept ConstReadModifier;                                         \
                                                                                                                       \
            /*!                                                                                                        \
             * \brief Deletes current item from collection and moves to next item.                                     \
             *                                                                                                         \
             * \invariant Cursor should not point to ending.                                                           \
             */                                                                                                        \
            Cursor &operator~ () noexcept DeleteModifier; /* NOLINT(bugprone-macro-parentheses) */                     \
                                                                                                                       \
            /*!                                                                                                        \
             * \brief Checks current item for changes. Then moves cursor to next item.                                 \
             * \invariant Cursor should not point to ending.                                                           \
             */                                                                                                        \
            Cursor &operator++ () noexcept; /* NOLINT(bugprone-macro-parentheses): Types can not be enclosed. */       \
                                                                                                                       \
            /*! Assigning cursors looks counter intuitive. */                                                          \
            EMERGENCE_DELETE_ASSIGNMENT (Cursor);                                                                      \
                                                                                                                       \
        private:                                                                                                       \
            friend class QueryClass;                                                                                   \
                                                                                                                       \
            explicit Cursor (BaseClass::Cursor _source,                                                                \
                             Emergence::Celerity::TrivialEventTriggerRow *_eventsOnRemove,                             \
                             Emergence::Celerity::ChangeTracker *_changeTracker) noexcept;                             \
                                                                                                                       \
            void EnterRecord () noexcept;                                                                              \
                                                                                                                       \
            void ExitRecord () noexcept;                                                                               \
                                                                                                                       \
            BaseClass::Cursor source;                                                                                  \
            [[maybe_unused]] Emergence::Celerity::TrivialEventTriggerRow *eventsOnRemove;                              \
            [[maybe_unused]] Emergence::Celerity::ChangeTracker *changeTracker;                                        \
        };                                                                                                             \
                                                                                                                       \
        QueryClass (const QueryClass &_other) noexcept;                                                                \
                                                                                                                       \
        QueryClass (QueryClass &&_other) noexcept;                                                                     \
                                                                                                                       \
        /*! \invariant There is no active cursors for this query. */                                                   \
        ~QueryClass () noexcept;                                                                                       \
                                                                                                                       \
        /*! \invariant There is no other cursors for ::GetTypeMapping type in registry. */                             \
        Cursor Execute (__VA_ARGS__) noexcept;                                                                         \
                                                                                                                       \
        /*! \see BaseClass::UnsafeFetchAccessToken */                                                                  \
        BaseClass::UnsafeFetchAccessToken AllowUnsafeFetchAccess () noexcept;                                          \
                                                                                                                       \
        /*! Assigning prepared queries looks counter-intuitive. */                                                     \
        EMERGENCE_DELETE_ASSIGNMENT (QueryClass);                                                                      \
                                                                                                                       \
    private:                                                                                                           \
        friend class TaskConstructor;                                                                                  \
                                                                                                                       \
        explicit QueryClass (BaseClass _source,                                                                        \
                             Emergence::Celerity::TrivialEventTriggerRow *_eventsOnRemove,                             \
                             Emergence::Celerity::ChangeTracker *_changeTracker) noexcept;                             \
                                                                                                                       \
        BaseClass source;                                                                                              \
        Emergence::Celerity::TrivialEventTriggerRow *eventsOnRemove;                                                   \
        Emergence::Celerity::ChangeTracker *changeTracker;                                                             \
    }

/// \brief Wraps Warehouse query, leaving both record edition and record removal available to user.
#define EMERGENCE_CELERITY_MODIFY_QUERY_WRAPPER_DECLARATION(QueryName, ...)                                            \
    EMERGENCE_CELERITY_WRITE_QUERY_WRAPPER_DECLARATION (Modify##QueryName, Emergence::Warehouse::Modify##QueryName, ,  \
                                                        = delete, , __VA_ARGS__)

/// \brief Wraps Warehouse query, leaving only record edition available to user. Removal is disabled.
#define EMERGENCE_CELERITY_EDIT_QUERY_WRAPPER_DECLARATION(QueryName, ...)                                              \
    EMERGENCE_CELERITY_WRITE_QUERY_WRAPPER_DECLARATION (Edit##QueryName, Emergence::Warehouse::Modify##QueryName, ,    \
                                                        = delete, = delete, __VA_ARGS__)

/// \brief Wraps Warehouse query, leaving only record removal and const read available to user. Edition is disabled.
#define EMERGENCE_CELERITY_REMOVE_QUERY_WRAPPER_DECLARATION(QueryName, ...)                                            \
    EMERGENCE_CELERITY_WRITE_QUERY_WRAPPER_DECLARATION (Remove##QueryName, Emergence::Warehouse::Modify##QueryName,    \
                                                        = delete, , , __VA_ARGS__)

#define EMERGENCE_CELERITY_WRITE_QUERY_COMMON_WRAPPER_IMPLEMENTATION(QueryClass, BaseClass, Editable)                  \
    QueryClass::Cursor::Cursor (QueryClass::Cursor &&_other) noexcept                                                  \
        : source (std::move (_other.source)),                                                                          \
          eventsOnRemove (_other.eventsOnRemove),                                                                      \
          changeTracker (_other.changeTracker)                                                                         \
    {                                                                                                                  \
        _other.eventsOnRemove = nullptr;                                                                               \
        _other.changeTracker = nullptr;                                                                                \
    }                                                                                                                  \
                                                                                                                       \
    QueryClass::Cursor::~Cursor () noexcept                                                                            \
    {                                                                                                                  \
        if constexpr (Editable)                                                                                        \
        {                                                                                                              \
            ExitRecord ();                                                                                             \
        }                                                                                                              \
    }                                                                                                                  \
                                                                                                                       \
    QueryClass::Cursor &QueryClass::Cursor::operator++ () noexcept                                                     \
    {                                                                                                                  \
        if constexpr (Editable)                                                                                        \
        {                                                                                                              \
            ExitRecord ();                                                                                             \
        }                                                                                                              \
                                                                                                                       \
        ++source;                                                                                                      \
        if constexpr (Editable)                                                                                        \
        {                                                                                                              \
            EnterRecord ();                                                                                            \
        }                                                                                                              \
                                                                                                                       \
        return *this;                                                                                                  \
    }                                                                                                                  \
                                                                                                                       \
    QueryClass::Cursor::Cursor (BaseClass::Cursor _source,                                                             \
                                Emergence::Celerity::TrivialEventTriggerRow *_eventsOnRemove,                          \
                                Emergence::Celerity::ChangeTracker *_changeTracker) noexcept                           \
        : source (std::move (_source)),                                                                                \
          eventsOnRemove (_eventsOnRemove),                                                                            \
          changeTracker (_changeTracker)                                                                               \
                                                                                                                       \
    {                                                                                                                  \
        if constexpr (Editable)                                                                                        \
        {                                                                                                              \
            EnterRecord ();                                                                                            \
        }                                                                                                              \
    }                                                                                                                  \
                                                                                                                       \
    void QueryClass::Cursor::EnterRecord () noexcept                                                                   \
    {                                                                                                                  \
        if (changeTracker)                                                                                             \
        {                                                                                                              \
            if (void *current = *source)                                                                               \
            {                                                                                                          \
                changeTracker->BeginEdition (current);                                                                 \
            }                                                                                                          \
        }                                                                                                              \
    }                                                                                                                  \
                                                                                                                       \
    void QueryClass::Cursor::ExitRecord () noexcept                                                                    \
    {                                                                                                                  \
        if (changeTracker)                                                                                             \
        {                                                                                                              \
            if (void *current = *source)                                                                               \
            {                                                                                                          \
                changeTracker->EndEdition (current);                                                                   \
            }                                                                                                          \
        }                                                                                                              \
    }                                                                                                                  \
                                                                                                                       \
    QueryClass::~QueryClass () noexcept = default;                                                                     \
                                                                                                                       \
    QueryClass::QueryClass (const QueryClass &_other) noexcept = default;                                              \
                                                                                                                       \
    /* NOLINTNEXTLINE(bugprone-macro-parentheses): We can't put parentheses here. */                                   \
    QueryClass::QueryClass (QueryClass &&_other) noexcept = default;                                                   \
                                                                                                                       \
    BaseClass::UnsafeFetchAccessToken QueryClass::AllowUnsafeFetchAccess () noexcept                                   \
    {                                                                                                                  \
        return source.AllowUnsafeFetchAccess ();                                                                       \
    }                                                                                                                  \
                                                                                                                       \
    QueryClass::QueryClass (BaseClass _source, Emergence::Celerity::TrivialEventTriggerRow *_eventsOnRemove,           \
                            Emergence::Celerity::ChangeTracker *_changeTracker) noexcept                               \
        : source (std::move (_source)),                                                                                \
          eventsOnRemove (_eventsOnRemove),                                                                            \
          changeTracker (_changeTracker)                                                                               \
    {                                                                                                                  \
    }

#define EMERGENCE_CELERITY_WRITE_CURSOR_EDITABLE_READ_IMPLEMENTATION(QueryClass)                                       \
    /* NOLINTNEXTLINE(bugprone-macro-parentheses): Types can not be enclosed. */                                       \
    void *QueryClass::Cursor::operator* () noexcept                                                                    \
    {                                                                                                                  \
        return *source;                                                                                                \
    }

#define EMERGENCE_CELERITY_WRITE_CURSOR_CONST_READ_IMPLEMENTATION(QueryClass)                                          \
    const void *QueryClass::Cursor::ReadConst () noexcept                                                              \
    {                                                                                                                  \
        return *source;                                                                                                \
    }

#define EMERGENCE_CELERITY_WRITE_CURSOR_REMOVE_IMPLEMENTATION(QueryClass)                                              \
    QueryClass::Cursor &QueryClass::Cursor::operator~ () noexcept                                                      \
    {                                                                                                                  \
        if (eventsOnRemove)                                                                                            \
        {                                                                                                              \
            if (void *current = *source)                                                                               \
            {                                                                                                          \
                for (Emergence::Celerity::TrivialEventTrigger & trigger : *eventsOnRemove)                             \
                {                                                                                                      \
                    trigger.Trigger (current);                                                                         \
                }                                                                                                      \
            }                                                                                                          \
        }                                                                                                              \
                                                                                                                       \
        ~source;                                                                                                       \
        return *this;                                                                                                  \
    }

/// \brief Implements all methods, declared by EMERGENCE_CELERITY_MODIFY_QUERY_WRAPPER_DECLARATION, except Execute.
/// \details Query parameters are custom, therefore it is difficult to cover Execute method with macro.
#define EMERGENCE_CELERITY_MODIFY_QUERY_WRAPPER_IMPLEMENTATION(QueryName)                                              \
    EMERGENCE_CELERITY_WRITE_QUERY_COMMON_WRAPPER_IMPLEMENTATION (Modify##QueryName,                                   \
                                                                  Emergence::Warehouse::Modify##QueryName, true)       \
    EMERGENCE_CELERITY_WRITE_CURSOR_EDITABLE_READ_IMPLEMENTATION (Modify##QueryName)                                   \
    EMERGENCE_CELERITY_WRITE_CURSOR_REMOVE_IMPLEMENTATION (Modify##QueryName)

/// \brief Implements all methods, declared by EMERGENCE_CELERITY_EDIT_QUERY_WRAPPER_DECLARATION, except Execute.
/// \details Query parameters are custom, therefore it is difficult to cover Execute method with macro.
#define EMERGENCE_CELERITY_EDIT_QUERY_WRAPPER_IMPLEMENTATION(QueryName)                                                \
    EMERGENCE_CELERITY_WRITE_QUERY_COMMON_WRAPPER_IMPLEMENTATION (Edit##QueryName,                                     \
                                                                  Emergence::Warehouse::Modify##QueryName, true)       \
    EMERGENCE_CELERITY_WRITE_CURSOR_EDITABLE_READ_IMPLEMENTATION (Edit##QueryName)

/// \brief Implements all methods, declared by EMERGENCE_CELERITY_REMOVE_QUERY_WRAPPER_DECLARATION, except Execute.
/// \details Query parameters are custom, therefore it is difficult to cover Execute method with macro.
#define EMERGENCE_CELERITY_REMOVE_QUERY_WRAPPER_IMPLEMENTATION(QueryName)                                              \
    EMERGENCE_CELERITY_WRITE_QUERY_COMMON_WRAPPER_IMPLEMENTATION (Remove##QueryName,                                   \
                                                                  Emergence::Warehouse::Modify##QueryName, false)      \
    EMERGENCE_CELERITY_WRITE_CURSOR_CONST_READ_IMPLEMENTATION (Remove##QueryName)                                      \
    EMERGENCE_CELERITY_WRITE_CURSOR_REMOVE_IMPLEMENTATION (Remove##QueryName)
