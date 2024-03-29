#pragma once

#include <CelerityRenderFoundationModelApi.hpp>

#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/Event/Macro.generated.hpp>
#include <Celerity/Standard/UniqueId.hpp>

namespace Emergence::Celerity
{
#define EventsApi CelerityRenderFoundationModelApi
EMERGENCE_CELERITY_EVENT1_DECLARATION (ViewportAddedNormalEvent, Memory::UniqueString, name);
EMERGENCE_CELERITY_EVENT1_DECLARATION (ViewportAddedCustomToNormalEvent, Memory::UniqueString, name);
EMERGENCE_CELERITY_EVENT1_DECLARATION (ViewportChangedNormalEvent, Memory::UniqueString, name);
EMERGENCE_CELERITY_EVENT1_DECLARATION (ViewportRemovedNormalEvent, Memory::UniqueString, name);
#undef EventsApi

CelerityRenderFoundationModelApi void RegisterRenderFoundationEvents (EventRegistrar &_registrar) noexcept;
} // namespace Emergence::Celerity
