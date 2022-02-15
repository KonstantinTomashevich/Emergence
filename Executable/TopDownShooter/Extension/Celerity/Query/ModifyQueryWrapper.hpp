#pragma once

#include <API/Common/Cursor.hpp>

#include <Celerity/Event/EventTrigger.hpp>

/// \brief Declares wrapper class for given Warehouse query.
/// \warning Query type specific methods, like GetKeyField, are omitted to simplify wrapping. These methods are intended
///          to be used when query purpose is not fully known, but Celerity end user usually knows it, because queries
///          are created by tasks, therefore this methods are not needed.
/// \details Macros are used instead of template to avoid increasing compilation time: these queries are likely to be
///          used in almost every task, therefore number of template instantiations per translation unit could be huge.
#define WAREHOUSE_MODIFY_QUERY_WRAPPER_DECLARATION(QueryClass, ...)                                                    \
    /** \brief Wrapper for Emergence::Warehouse::QueryClass, that integrates insertion with Celerity events. */        \
    class QueryClass final                                                                                             \
    {                                                                                                                  \
    public:                                                                                                            \
        class Cursor final                                                                                             \
        {                                                                                                              \
        public:                                                                                                        \
            EMERGENCE_EDIT_CURSOR_OPERATIONS (Cursor);                                                                 \
                                                                                                                       \
        private:                                                                                                       \
            friend class QueryClass;                                                                                   \
                                                                                                                       \
            explicit Cursor (Emergence::Warehouse::QueryClass::Cursor _source,                                         \
                             Emergence::Celerity::TrivialEventTriggerRow *_eventsOnRemove,                             \
                             Emergence::Celerity::ChangeTracker *_changeTracker) noexcept;                             \
                                                                                                                       \
            void EnterRecord () noexcept;                                                                              \
                                                                                                                       \
            void ExitRecord () noexcept;                                                                               \
                                                                                                                       \
            Emergence::Warehouse::QueryClass::Cursor source;                                                           \
            Emergence::Celerity::TrivialEventTriggerRow *eventsOnRemove;                                               \
            Emergence::Celerity::ChangeTracker *changeTracker;                                                         \
        };                                                                                                             \
                                                                                                                       \
        EMERGENCE_EDITABLE_PREPARED_QUERY_OPERATIONS (QueryClass, Cursor, __VA_ARGS__);                                \
                                                                                                                       \
    private:                                                                                                           \
        friend class TaskConstructor;                                                                                  \
                                                                                                                       \
        explicit QueryClass (Emergence::Warehouse::QueryClass _source,                                                 \
                             Emergence::Celerity::TrivialEventTriggerRow *_eventsOnRemove,                             \
                             Emergence::Celerity::ChangeTracker *_changeTracker) noexcept;                             \
                                                                                                                       \
        Emergence::Warehouse::QueryClass source;                                                                       \
        Emergence::Celerity::TrivialEventTriggerRow *eventsOnRemove;                                                   \
        Emergence::Celerity::ChangeTracker *changeTracker;                                                             \
    }

/// \brief Implements all methods, declared by WAREHOUSE_MODIFY_QUERY_WRAPPER_DECLARATION, except Execute.
/// \details Query parameters are custom, therefore it is difficult to cover Execute method with macro.
#define WAREHOUSE_MODIFY_QUERY_WRAPPER_IMPLEMENTATION(QueryClass)                                                      \
    QueryClass::Cursor::Cursor (QueryClass::Cursor &&_other) noexcept                                                  \
        : source (std::move (_other.source)),                                                                          \
          eventsOnRemove (_other.eventsOnRemove),                                                                      \
          changeTracker (_other.changeTracker)                                                                         \
    {                                                                                                                  \
        _other.eventsOnRemove = nullptr;                                                                               \
        _other.changeTracker = nullptr;                                                                                \
    }                                                                                                                  \
                                                                                                                       \
    void *QueryClass::Cursor::operator* () noexcept                                                                    \
    {                                                                                                                  \
        return *source;                                                                                                \
    }                                                                                                                  \
                                                                                                                       \
    QueryClass::Cursor::~Cursor () noexcept                                                                            \
    {                                                                                                                  \
        ExitRecord ();                                                                                                 \
    }                                                                                                                  \
                                                                                                                       \
    QueryClass::Cursor &QueryClass::Cursor::operator++ () noexcept                                                     \
    {                                                                                                                  \
        ExitRecord ();                                                                                                 \
        ++source;                                                                                                      \
        EnterRecord ();                                                                                                \
        return *this;                                                                                                  \
    }                                                                                                                  \
                                                                                                                       \
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
    }                                                                                                                  \
                                                                                                                       \
    QueryClass::Cursor::Cursor (Emergence::Warehouse::QueryClass::Cursor _source,                                      \
                                Emergence::Celerity::TrivialEventTriggerRow *_eventsOnRemove,                          \
                                Emergence::Celerity::ChangeTracker *_changeTracker) noexcept                           \
        : source (std::move (_source)),                                                                                \
          eventsOnRemove (_eventsOnRemove),                                                                            \
          changeTracker (_changeTracker)                                                                               \
                                                                                                                       \
    {                                                                                                                  \
        EnterRecord ();                                                                                                \
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
    StandardLayout::Mapping QueryClass::GetTypeMapping () const noexcept                                               \
    {                                                                                                                  \
        return source.GetTypeMapping ();                                                                               \
    }                                                                                                                  \
                                                                                                                       \
    QueryClass::QueryClass (const QueryClass &_other) noexcept = default;                                              \
                                                                                                                       \
    QueryClass::QueryClass (QueryClass &&_other) noexcept = default;                                                   \
                                                                                                                       \
    void QueryClass::AddCustomVisualization (VisualGraph::Graph &_graph) const noexcept                                \
    {                                                                                                                  \
        source.AddCustomVisualization (_graph);                                                                        \
    }                                                                                                                  \
                                                                                                                       \
    QueryClass::QueryClass (Emergence::Warehouse::QueryClass _source,                                                  \
                            Emergence::Celerity::TrivialEventTriggerRow *_eventsOnRemove,                              \
                            Emergence::Celerity::ChangeTracker *_changeTracker) noexcept                               \
        : source (std::move (_source)),                                                                                \
          eventsOnRemove (_eventsOnRemove),                                                                            \
          changeTracker (_changeTracker)                                                                               \
    {                                                                                                                  \
    }
