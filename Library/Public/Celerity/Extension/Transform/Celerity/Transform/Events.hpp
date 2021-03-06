#pragma once

#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/Event/Macro.generated.hpp>
#include <Celerity/Standard/UniqueId.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_EVENT1_DECLARATION (Transform3dComponentRemovedFixedEvent, UniqueId, objectId);
EMERGENCE_CELERITY_EVENT1_DECLARATION (Transform3dComponentRemovedNormalEvent, UniqueId, objectId);
EMERGENCE_CELERITY_EVENT1_DECLARATION (Transform3dComponentRemovedFixedToNormalEvent, UniqueId, objectId);

void RegisterTransformEvents (EventRegistrar &_registrar) noexcept;
} // namespace Emergence::Celerity
