#pragma once

#include <CelerityUIModelApi.hpp>

#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/Event/Macro.generated.hpp>
#include <Celerity/Standard/UniqueId.hpp>

#include <Memory/UniqueString.hpp>

namespace Emergence::Celerity
{
#define EventsApi CelerityUIModelApi
EMERGENCE_CELERITY_EVENT1_DECLARATION (UINodeRemovedNormalEvent, UniqueId, nodeId);
#undef EventsApi

CelerityUIModelApi void RegisterUIEvents (EventRegistrar &_registrar) noexcept;
} // namespace Emergence::Celerity
