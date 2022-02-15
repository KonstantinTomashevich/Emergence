#pragma once

#include <Celerity/Event/EventTrigger.hpp>
#include <Celerity/World.hpp>

namespace Emergence::Celerity
{
struct ClearableEventSeed
{
    StandardLayout::Mapping eventType;
    EventRoute route;
};

/// \invariant Records of ::recordType were added using InsertLongTermQuery. Short term objects are not supported,
///            because of event insertion prepared query construction problem: if prepared query triggers events,
///            it must be constructed after all relevant automated events are constructed, therefore additional
///            ordering step would be required. Also, short term objects are usually events, therefore usually
///            there is no need to track their addition and removal.
struct TrivialAutomatedEventSeed : public ClearableEventSeed
{
    StandardLayout::Mapping trackedType;
    Container::Vector<CopyOutField> copyOut;
};

/// \invariant Records of ::recordType are either singletons or long term objects.
///            For explanation check same invariant in ::TrivialAutomatedEventSeed.
struct OnChangeAutomatedEventSeed : public ClearableEventSeed
{
    StandardLayout::Mapping trackedType;
    Container::Vector<StandardLayout::FieldId> trackedFields;

    /// \invariant Only tracked fields are allowed to be copied from initial state.
    Container::Vector<CopyOutField> copyOutOfInitial;

    Container::Vector<CopyOutField> copyOutOfChanged;
};

class EventRegistrar final
{
public:
    EventRegistrar (World *_world) noexcept;

    EventRegistrar (const EventRegistrar &_other) = delete;

    EventRegistrar (EventRegistrar &&_other) noexcept;

    ~EventRegistrar () noexcept;

    void CustomEvent (const ClearableEventSeed &_seed) noexcept;

    void OnAddEvent (const TrivialAutomatedEventSeed &_seed) noexcept;

    void OnRemoveEvent (const TrivialAutomatedEventSeed &_seed) noexcept;

    void OnChangeEvent (const OnChangeAutomatedEventSeed &_seed) noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (EventRegistrar);

private:
    void AssertEventUniqueness (const StandardLayout::Mapping &_type) const noexcept;

    World *world;
};
} // namespace Emergence::Celerity
