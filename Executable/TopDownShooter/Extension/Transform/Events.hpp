#pragma once

#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/Event/Macro.generated.hpp>
#include <Celerity/Standard/UniqueId.hpp>

namespace Emergence::Transform
{
EMERGENCE_CELERITY_EVENT1_DECLARATION (Transform3dComponentRemovedFixedEvent, Celerity::UniqueId, objectId);

void RegisterEvents (Celerity::EventRegistrar &_registrar) noexcept;
} // namespace Emergence::Transform
