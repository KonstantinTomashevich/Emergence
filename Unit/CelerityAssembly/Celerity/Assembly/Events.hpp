#pragma once

#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/Event/Macro.generated.hpp>
#include <Celerity/Standard/UniqueId.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_EVENT1_DECLARATION (AssemblyFinishedFixedEvent, UniqueId, objectId);
EMERGENCE_CELERITY_EVENT1_DECLARATION (AssemblyFinishedNormalEvent, UniqueId, objectId);

void RegisterAssemblyEvents (EventRegistrar &_registrar) noexcept;
} // namespace Emergence::Celerity
