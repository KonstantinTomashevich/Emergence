#pragma once

#include <CelerityApi.hpp>

#include <Celerity/Event/EventTrigger.hpp>
#include <Celerity/World.hpp>

namespace Emergence::Celerity
{
/// \brief Base seed for all events, that are managed by Celerity.
struct CelerityApi ClearableEventSeed
{
    StandardLayout::Mapping eventType;
    EventRoute route;
};

/// \brief Seed for trivial (on add, on remove) automated events.
/// \invariant Records of ::trackedType were added using InsertLongTermQuery. Short term objects are not supported,
///            because of event insertion prepared query construction problem: if prepared query triggers events,
///            it must be constructed after all relevant automated events are constructed, therefore additional
///            ordering step would be required. Also, short term objects are usually events, therefore usually
///            there is no need to track their addition and removal.
struct CelerityApi TrivialAutomatedEventSeed : public ClearableEventSeed
{
    StandardLayout::Mapping trackedType;
    Container::Vector<CopyOutField> copyOut;
};

/// \brief Seed for automated event, that is fired when any record field from ::trackedFields is changed.
/// \invariant Records of ::recordType are either singletons or long term objects.
///            For explanation check same invariant in ::TrivialAutomatedEventSeed.
struct CelerityApi OnChangeAutomatedEventSeed : public ClearableEventSeed
{
    StandardLayout::Mapping trackedType;
    Container::Vector<StandardLayout::FieldId> trackedFields;

    /// \invariant Only tracked fields are allowed to be copied from initial state.
    Container::Vector<CopyOutField> copyOutOfInitial;

    Container::Vector<CopyOutField> copyOutOfChanged;
};

/// \brief Interface for adding events into World.
/// \invariant All events for the single world must be registered though single registrar!
class CelerityApi EventRegistrar final
{
public:
    /// \brief Constructs event registrar for given world and start registration routine.
    /// \details `unsafe` flag informs  logic that it is okay that world already has other event scheme:
    ///          user is aware of that and will manually update pipelines accordingly.
    EventRegistrar (World *_world, bool unsafe = false) noexcept;

    EventRegistrar (const EventRegistrar &_other) = delete;

    EventRegistrar (EventRegistrar &&_other) noexcept;

    ~EventRegistrar () noexcept;

    /// \brief Registers event with custom firing logic.
    /// \invariant Same struct can not be used to register several events!
    void CustomEvent (const ClearableEventSeed &_seed) noexcept;

    /// \brief Registers event that is fired after object creation.
    /// \invariant Same struct can not be used to register several events!
    void OnAddEvent (const TrivialAutomatedEventSeed &_seed) noexcept;

    /// \brief Registers event that is fired after object removal.
    /// \invariant Same struct can not be used to register several events!
    void OnRemoveEvent (const TrivialAutomatedEventSeed &_seed) noexcept;

    /// \brief Registers event that is fired after specified fields of an object are changed.
    /// \invariant Same struct can not be used to register several events!
    void OnChangeEvent (const OnChangeAutomatedEventSeed &_seed) noexcept;

    /// There is no need to move assign event registrars.
    EMERGENCE_DELETE_ASSIGNMENT (EventRegistrar);

private:
    void AssertEventUniqueness (const StandardLayout::Mapping &_type) const noexcept;

    World::EventScheme &SelectScheme (EventRoute _route) noexcept;

    World *world;
};
} // namespace Emergence::Celerity
