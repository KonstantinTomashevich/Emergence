#pragma once

#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/Event/Macro.generated.hpp>
#include <Celerity/Standard/UniqueId.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_EVENT2_DECLARATION (Sprite2dAddedNormalEvent, UniqueId, objectId, UniqueId, spriteId);
EMERGENCE_CELERITY_EVENT2_DECLARATION (Sprite2dSizeChangedNormalEvent, UniqueId, objectId, UniqueId, spriteId);
EMERGENCE_CELERITY_EVENT1_DECLARATION (Sprite2dRemovedNormalEvent, UniqueId, objectId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (ViewportAddedNormalEvent, Memory::UniqueString, name);
EMERGENCE_CELERITY_EVENT1_DECLARATION (ViewportAddedCustomToNormalEvent, Memory::UniqueString, name);
EMERGENCE_CELERITY_EVENT1_DECLARATION (ViewportChangedNormalEvent, Memory::UniqueString, name);

void RegisterRender2dEvents (EventRegistrar &_registrar) noexcept;
} // namespace Emergence::Celerity
