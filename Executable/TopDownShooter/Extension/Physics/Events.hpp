#pragma once

#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/Event/Macro.generated.hpp>
#include <Celerity/Standard/UniqueId.hpp>

#include <Memory/UniqueString.hpp>

namespace Emergence::Physics
{
EMERGENCE_CELERITY_EVENT1_DECLARATION (DynamicsMaterialAddedEvent, Memory::UniqueString, id);

EMERGENCE_CELERITY_EVENT1_DECLARATION (DynamicsMaterialChangedEvent, Memory::UniqueString, id);

EMERGENCE_CELERITY_EVENT1_DECLARATION (DynamicsMaterialRemovedEvent, Memory::UniqueString, id);

EMERGENCE_CELERITY_EVENT1_DECLARATION (CollisionShapeComponentAddedEvent, Celerity::UniqueId, shapeId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (CollisionShapeComponentMaterialChangedEvent, Celerity::UniqueId, shapeId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (CollisionShapeComponentGeometryChangedEvent, Celerity::UniqueId, shapeId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (CollisionShapeComponentAttributesChangedEvent, Celerity::UniqueId, shapeId);

EMERGENCE_CELERITY_EVENT2_DECLARATION (
    CollisionShapeComponentRemovedEvent, Celerity::UniqueId, objectId, void *, implementationHandle);

void RegisterEvents (Celerity::EventRegistrar &_registrar) noexcept;
} // namespace Emergence::Physics
