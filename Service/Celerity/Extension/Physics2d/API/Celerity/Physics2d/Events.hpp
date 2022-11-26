#pragma once

#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/Event/Macro.generated.hpp>
#include <Celerity/Standard/UniqueId.hpp>

#include <Memory/UniqueString.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_EVENT0_DECLARATION (PhysicsWorld2dConfigurationChanged);

EMERGENCE_CELERITY_EVENT1_DECLARATION (DynamicsMaterial2dAddedFixedEvent, Memory::UniqueString, id);

EMERGENCE_CELERITY_EVENT1_DECLARATION (DynamicsMaterial2dAddedCustomToFixedEvent, Memory::UniqueString, id);

EMERGENCE_CELERITY_EVENT1_DECLARATION (DynamicsMaterial2dChangedEvent, Memory::UniqueString, id);

EMERGENCE_CELERITY_EVENT1_DECLARATION (DynamicsMaterial2dRemovedEvent, Memory::UniqueString, id);

EMERGENCE_CELERITY_EVENT1_DECLARATION (CollisionShape2dComponentAddedFixedEvent, UniqueId, shapeId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (CollisionShape2dComponentAddedCustomToFixedEvent, UniqueId, shapeId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (CollisionShape2dComponentMaterialChangedEvent, UniqueId, shapeId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (CollisionShape2dComponentGeometryChangedEvent, UniqueId, shapeId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (CollisionShape2dComponentAttributesChangedEvent, UniqueId, shapeId);

EMERGENCE_CELERITY_EVENT2_DECLARATION (
    CollisionShape2dComponentRemovedEvent, UniqueId, objectId, void *, implementationHandle);

EMERGENCE_CELERITY_EVENT1_DECLARATION (RigidBody2dComponentAddedFixedEvent, UniqueId, objectId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (RigidBody2dComponentAddedCustomToFixedEvent, UniqueId, objectId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (RigidBody2dComponentMassInvalidatedEvent, UniqueId, objectId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (RigidBody2dComponentRemovedEvent, UniqueId, objectId);

EMERGENCE_CELERITY_EVENT4_DECLARATION (Contact2dFoundEvent,
                                       UniqueId,
                                       firstObjectId,
                                       UniqueId,
                                       firstShapeId,
                                       UniqueId,
                                       secondObjectId,
                                       UniqueId,
                                       secondShapeId);

EMERGENCE_CELERITY_EVENT4_DECLARATION (Contact2dLostEvent,
                                       UniqueId,
                                       firstObjectId,
                                       UniqueId,
                                       firstShapeId,
                                       UniqueId,
                                       secondObjectId,
                                       UniqueId,
                                       secondShapeId);

EMERGENCE_CELERITY_EVENT4_DECLARATION (Trigger2dEnteredEvent,
                                       UniqueId,
                                       triggerObjectId,
                                       UniqueId,
                                       triggerShapeId,
                                       UniqueId,
                                       intruderObjectId,
                                       UniqueId,
                                       intruderShapeId);

EMERGENCE_CELERITY_EVENT4_DECLARATION (Trigger2dExitedEvent,
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
