#pragma once

#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/Event/Macro.generated.hpp>
#include <Celerity/Standard/UniqueId.hpp>

#include <Memory/UniqueString.hpp>

namespace Emergence::Physics
{
EMERGENCE_CELERITY_EVENT0_DECLARATION(PhysicsWorldConfigurationChanged);

EMERGENCE_CELERITY_EVENT1_DECLARATION (DynamicsMaterialAddedEvent, Memory::UniqueString, id);

EMERGENCE_CELERITY_EVENT1_DECLARATION (DynamicsMaterialChangedEvent, Memory::UniqueString, id);

EMERGENCE_CELERITY_EVENT2_DECLARATION (
    DynamicsMaterialRemovedEvent, Memory::UniqueString, id, void *, implementationHandle);

EMERGENCE_CELERITY_EVENT1_DECLARATION (CollisionShapeComponentAddedEvent, Celerity::UniqueId, shapeId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (CollisionShapeComponentMaterialChangedEvent, Celerity::UniqueId, shapeId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (CollisionShapeComponentGeometryChangedEvent, Celerity::UniqueId, shapeId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (CollisionShapeComponentAttributesChangedEvent, Celerity::UniqueId, shapeId);

EMERGENCE_CELERITY_EVENT2_DECLARATION (
    CollisionShapeComponentRemovedEvent, Celerity::UniqueId, objectId, void *, implementationHandle);

EMERGENCE_CELERITY_EVENT1_DECLARATION (RigidBodyComponentAddedEvent, Celerity::UniqueId, objectId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (RigidBodyComponentMassInvalidatedEvent, Celerity::UniqueId, objectId);

EMERGENCE_CELERITY_EVENT2_DECLARATION (
    RigidBodyComponentRemovedEvent, Celerity::UniqueId, objectId, void *, implementationHandle);

EMERGENCE_CELERITY_EVENT5_DECLARATION (ContactFoundEvent,
                                       Celerity::UniqueId,
                                       firstObjectId,
                                       Celerity::UniqueId,
                                       firstShapeId,
                                       Celerity::UniqueId,
                                       secondObjectId,
                                       Celerity::UniqueId,
                                       secondShapeId,
                                       bool,
                                       initialContact);

EMERGENCE_CELERITY_EVENT4_DECLARATION (ContactPersistsEvent,
                                       Celerity::UniqueId,
                                       firstObjectId,
                                       Celerity::UniqueId,
                                       firstShapeId,
                                       Celerity::UniqueId,
                                       secondObjectId,
                                       Celerity::UniqueId,
                                       secondShapeId);

EMERGENCE_CELERITY_EVENT5_DECLARATION (ContactLostEvent,
                                       Celerity::UniqueId,
                                       firstObjectId,
                                       Celerity::UniqueId,
                                       firstShapeId,
                                       Celerity::UniqueId,
                                       secondObjectId,
                                       Celerity::UniqueId,
                                       secondShapeId,
                                       bool,
                                       lastContact);

EMERGENCE_CELERITY_EVENT4_DECLARATION (TriggerEnteredEvent,
                                       Celerity::UniqueId,
                                       triggerObjectId,
                                       Celerity::UniqueId,
                                       triggerShapeId,
                                       Celerity::UniqueId,
                                       intruderObjectId,
                                       Celerity::UniqueId,
                                       intruderShapeId);

EMERGENCE_CELERITY_EVENT4_DECLARATION (TriggerExitedEvent,
                                       Celerity::UniqueId,
                                       triggerObjectId,
                                       Celerity::UniqueId,
                                       triggerShapeId,
                                       Celerity::UniqueId,
                                       intruderObjectId,
                                       Celerity::UniqueId,
                                       intruderShapeId);

void RegisterEvents (Celerity::EventRegistrar &_registrar) noexcept;
} // namespace Emergence::Physics
