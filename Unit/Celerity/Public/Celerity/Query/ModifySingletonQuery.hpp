#pragma once

#include <CelerityApi.hpp>

#include <Celerity/Event/EventTrigger.hpp>

#include <Warehouse/ModifySingletonQuery.hpp>

namespace Emergence::Celerity
{
/// \brief Wrapper for Warehouse::ModifySingletonQuery, that integrates modification with Celerity events.
class CelerityApi ModifySingletonQuery final
{
public:
    /// \brief Provides readwrite access to singleton instance.
    class CelerityApi Cursor final
    {
    public:
        Cursor (const Cursor &_other) = delete;

        Cursor (Cursor &&_other) noexcept;

        ~Cursor () noexcept;

        /// \return Singleton instance.
        void *operator* () const noexcept;

        /// Assigning cursors looks counter intuitive.
        EMERGENCE_DELETE_ASSIGNMENT (Cursor);

    private:
        /// Prepared query constructs its cursors.
        friend class ModifySingletonQuery;

        explicit Cursor (Warehouse::ModifySingletonQuery::Cursor _source,
                         OnChangeEventTriggerInstanceRow *_onChangeEvents,
                         ChangeTracker *_changeTracker) noexcept;

        Warehouse::ModifySingletonQuery::Cursor source;
        OnChangeEventTriggerInstanceRow *onChangeEvents;
        ChangeTracker *changeTracker;
    };

    EMERGENCE_EDITABLE_PREPARED_QUERY_OPERATIONS (ModifySingletonQuery, Cursor);

private:
    /// TaskConstructor constructs prepared queries wrappers.
    friend class TaskConstructor;

    explicit ModifySingletonQuery (Warehouse::ModifySingletonQuery _source,
                                   OnChangeEventTriggerInstanceRow *_onChangeEvents,
                                   ChangeTracker *_changeTracker) noexcept;

    Warehouse::ModifySingletonQuery source;
    OnChangeEventTriggerInstanceRow *onChangeEvents;
    ChangeTracker *changeTracker;
};
} // namespace Emergence::Celerity
