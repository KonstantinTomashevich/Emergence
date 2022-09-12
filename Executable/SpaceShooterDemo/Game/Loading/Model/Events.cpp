#include <Loading/Model/Events.hpp>

EMERGENCE_CELERITY_EVENT0_IMPLEMENTATION (LoadingFinishedEvent)

void RegisterLoadingEvents (Emergence::Celerity::EventRegistrar &_registrar) noexcept
{
    _registrar.CustomEvent ({LoadingFinishedEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::CUSTOM});
}
