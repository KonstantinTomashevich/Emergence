#pragma once

#include <CelerityApi.hpp>

#include <Celerity/Event/EventTrigger.hpp>

#include <Warehouse/InsertLongTermQuery.hpp>

namespace Emergence::Celerity
{
/// \brief Wrapper for Warehouse::InsertLongTermQuery, that integrates insertion with Celerity events.
class CelerityApi InsertLongTermQuery final
{
public:
    /// \brief Allows user to allocate new objects and insert them into registry.
    class CelerityApi Cursor final
    {
    public:
        Cursor (const Cursor &_other) = delete;

        Cursor (Cursor &&_other) noexcept;

        /// \invariant Previously allocated object must be initialized before cursor destruction.
        ~Cursor () noexcept;

        /// \return Pointer to memory, allocated for the new object.
        /// \details Object default constructor will be called, if it has one.
        /// \invariant Previously allocated object must be initialized before next call.
        void *operator++ () noexcept;

        /// Assigning cursors looks counter intuitive.
        EMERGENCE_DELETE_ASSIGNMENT (Cursor);

    private:
        /// Prepared query constructs its cursors.
        friend class InsertLongTermQuery;

        explicit Cursor (Warehouse::InsertLongTermQuery::Cursor _source,
                         TrivialEventTriggerInstanceRow *_eventsOnAdd) noexcept;

        void FireEvent () noexcept;

        Warehouse::InsertLongTermQuery::Cursor source;
        TrivialEventTriggerInstanceRow *eventsOnAdd;
        void *current = nullptr;
    };

    EMERGENCE_EDITABLE_PREPARED_QUERY_OPERATIONS (InsertLongTermQuery, Cursor);

private:
    /// TaskConstructor constructs prepared queries wrappers.
    friend class TaskConstructor;

    explicit InsertLongTermQuery (Warehouse::InsertLongTermQuery _source,
                                  TrivialEventTriggerInstanceRow *_eventsOnAdd) noexcept;

    Warehouse::InsertLongTermQuery source;
    TrivialEventTriggerInstanceRow *eventsOnAdd;
};
} // namespace Emergence::Celerity
