#pragma once

#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/Event/Macro.generated.hpp>

EMERGENCE_CELERITY_EVENT0_DECLARATION (LoadingFinishedEvent);

void RegisterLoadingEvents (Emergence::Celerity::EventRegistrar &_registrar) noexcept;
