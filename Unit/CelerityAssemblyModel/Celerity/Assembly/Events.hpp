#pragma once

#include <CelerityAssemblyModelApi.hpp>

#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/Event/Macro.generated.hpp>
#include <Celerity/Standard/UniqueId.hpp>

namespace Emergence::Celerity
{
#define EventsApi CelerityAssemblyModelApi
EMERGENCE_CELERITY_EVENT1_DECLARATION (AssemblyFinishedFixedEvent, UniqueId, objectId);
EMERGENCE_CELERITY_EVENT1_DECLARATION (AssemblyFinishedNormalEvent, UniqueId, objectId);
#undef EventsApi

CelerityAssemblyModelApi void RegisterAssemblyEvents (EventRegistrar &_registrar) noexcept;
} // namespace Emergence::Celerity
