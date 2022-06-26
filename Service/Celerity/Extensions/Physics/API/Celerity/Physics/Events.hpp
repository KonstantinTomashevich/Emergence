#pragma once

#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/Event/Macro.generated.hpp>
#include <Celerity/Standard/UniqueId.hpp>

#include <Memory/UniqueString.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_EVENT0_DECLARATION (PhysicsWorldConfigurationChanged);

EMERGENCE_CELERITY_EVENT1_DECLARATION (DynamicsMaterialAddedFixedEvent, Memory::UniqueString, id);

EMERGENCE_CELERITY_EVENT1_DECLARATION (DynamicsMaterialAddedCustomToFixedEvent, Memory::UniqueString, id);

EMERGENCE_CELERITY_EVENT1_DECLARATION (DynamicsMaterialChangedEvent, Memory::UniqueString, id);

EMERGENCE_CELERITY_EVENT2_DECLARATION (
    DynamicsMaterialRemovedEvent, Memory::UniqueString, id, void *, implementationHandle);

EMERGENCE_CELERITY_EVENT1_DECLARATION (CollisionShapeComponentAddedFixedEvent, UniqueId, shapeId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (CollisionShapeComponentAddedCustomToFixedEvent, UniqueId, shapeId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (CollisionShapeComponentMaterialChangedEvent, UniqueId, shapeId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (CollisionShapeComponentGeometryChangedEvent, UniqueId, shapeId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (CollisionShapeComponentAttributesChangedEvent, UniqueId, shapeId);

EMERGENCE_CELERITY_EVENT2_DECLARATION (
    CollisionShapeComponentRemovedEvent, UniqueId, objectId, void *, implementationHandle);

EMERGENCE_CELERITY_EVENT1_DECLARATION (RigidBodyComponentAddedFixedEvent, UniqueId, objectId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (RigidBodyComponentAddedCustomToFixedEvent, UniqueId, objectId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (RigidBodyComponentMassInvalidatedEvent, UniqueId, objectId);

EMERGENCE_CELERITY_EVENT2_DECLARATION (
    RigidBodyComponentRemovedEvent, UniqueId, objectId, void *, implementationHandle);

EMERGENCE_CELERITY_EVENT5_DECLARATION (ContactFoundEvent,
                                       UniqueId,
                                       firstObjectId,
                                       UniqueId,
                                       firstShapeId,
                                       UniqueId,
                                       secondObjectId,
                                       UniqueId,
                                       secondShapeId,
                                       bool,
                                       initialContact);

EMERGENCE_CELERITY_EVENT4_DECLARATION (ContactPersistsEvent,
                                       UniqueId,
                                       firstObjectId,
                                       UniqueId,
                                       firstShapeId,
                                       UniqueId,
                                       secondObjectId,
                                       UniqueId,
                                       secondShapeId);

EMERGENCE_CELERITY_EVENT5_DECLARATION (ContactLostEvent,
                                       UniqueId,
                                       firstObjectId,
                                       UniqueId,
                                       firstShapeId,
                                       UniqueId,
                                       secondObjectId,
                                       UniqueId,
                                       secondShapeId,
                                       bool,
                                       lastContact);

EMERGENCE_CELERITY_EVENT4_DECLARATION (TriggerEnteredEvent,
                                       UniqueId,
                                       triggerObjectId,
                                       UniqueId,
                                       triggerShapeId,
                                       UniqueId,
                                       intruderObjectId,
                                       UniqueId,
                                       intruderShapeId);

EMERGENCE_CELERITY_EVENT4_DECLARATION (TriggerExitedEvent,
                                       UniqueId,
                                       triggerObjectId,
                                       UniqueId,
                                       triggerShapeId,
                                       UniqueId,
                                       intruderObjectId,
                                       UniqueId,
                                       intruderShapeId);

void RegisterPhysicsEvents (EventRegistrar &_registrar) noexcept;
} // namespace Emergence::Celerity
