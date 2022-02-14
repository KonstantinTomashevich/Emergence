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

struct TrivialAutomatedEventSeed : public ClearableEventSeed
{
    StandardLayout::Mapping recordType;
    Container::Vector<CopyOutField> copyOut;
};

struct OnChangeAutomatedEventSeed : public ClearableEventSeed
{
    StandardLayout::Mapping recordType;
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
