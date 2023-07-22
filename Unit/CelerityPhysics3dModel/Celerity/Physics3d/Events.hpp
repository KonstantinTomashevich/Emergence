#pragma once

#include <CelerityPhysics3dModelApi.hpp>

#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/Event/Macro.generated.hpp>
#include <Celerity/Standard/UniqueId.hpp>

#include <Memory/UniqueString.hpp>

namespace Emergence::Celerity
{
#define EventsApi CelerityPhysics3dModelApi
EMERGENCE_CELERITY_EVENT0_DECLARATION (PhysicsWorld3dConfigurationChanged);

EMERGENCE_CELERITY_EVENT1_DECLARATION (DynamicsMaterial3dAddedFixedEvent, Memory::UniqueString, id);

EMERGENCE_CELERITY_EVENT1_DECLARATION (DynamicsMaterial3dAddedCustomToFixedEvent, Memory::UniqueString, id);

EMERGENCE_CELERITY_EVENT1_DECLARATION (DynamicsMaterial3dChangedEvent, Memory::UniqueString, id);

EMERGENCE_CELERITY_EVENT2_DECLARATION (
    DynamicsMaterial3dRemovedEvent, Memory::UniqueString, id, void *, implementationHandle);

EMERGENCE_CELERITY_EVENT1_DECLARATION (CollisionShape3dComponentAddedFixedEvent, UniqueId, shapeId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (CollisionShape3dComponentAddedCustomToFixedEvent, UniqueId, shapeId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (CollisionShape3dComponentMaterialChangedEvent, UniqueId, shapeId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (CollisionShape3dComponentGeometryChangedEvent, UniqueId, shapeId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (CollisionShape3dComponentAttributesChangedEvent, UniqueId, shapeId);

EMERGENCE_CELERITY_EVENT2_DECLARATION (
    CollisionShape3dComponentRemovedEvent, UniqueId, objectId, void *, implementationHandle);

EMERGENCE_CELERITY_EVENT1_DECLARATION (RigidBody3dComponentAddedFixedEvent, UniqueId, objectId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (RigidBody3dComponentAddedCustomToFixedEvent, UniqueId, objectId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (RigidBody3dComponentMassInvalidatedEvent, UniqueId, objectId);

EMERGENCE_CELERITY_EVENT2_DECLARATION (
    RigidBody3dComponentRemovedEvent, UniqueId, objectId, void *, implementationHandle);

EMERGENCE_CELERITY_EVENT5_DECLARATION (Contact3dFoundEvent,
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

EMERGENCE_CELERITY_EVENT4_DECLARATION (Contact3dPersistsEvent,
                                       UniqueId,
                                       firstObjectId,
                                       UniqueId,
                                       firstShapeId,
                                       UniqueId,
                                       secondObjectId,
                                       UniqueId,
                                       secondShapeId);

EMERGENCE_CELERITY_EVENT5_DECLARATION (Contact3dLostEvent,
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

EMERGENCE_CELERITY_EVENT4_DECLARATION (Trigger3dEnteredEvent,
                                       UniqueId,
                                       triggerObjectId,
                                       UniqueId,
                                       triggerShapeId,
                                       UniqueId,
                                       intruderObjectId,
                                       UniqueId,
                                       intruderShapeId);

EMERGENCE_CELERITY_EVENT4_DECLARATION (Trigger3dExitedEvent,
                                       UniqueId,
                                       triggerObjectId,
                                       UniqueId,
                                       triggerShapeId,
                                       UniqueId,
                                       intruderObjectId,
                                       UniqueId,
                                       intruderShapeId);
#undef EventsApi

CelerityPhysics3dModelApi void RegisterPhysicsEvents (EventRegistrar &_registrar) noexcept;
} // namespace Emergence::Celerity
