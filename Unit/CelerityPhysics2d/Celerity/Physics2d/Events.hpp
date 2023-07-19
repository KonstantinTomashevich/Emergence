#pragma once

#include <CelerityPhysics2dApi.hpp>

#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/Event/Macro.generated.hpp>
#include <Celerity/Standard/UniqueId.hpp>

#include <Memory/UniqueString.hpp>

namespace Emergence::Celerity
{
#define EventsApi CelerityPhysics2dApi
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

EMERGENCE_CELERITY_EVENT3_DECLARATION (
    CollisionShape2dComponentRemovedEvent, UniqueId, objectId, UniqueId, shapeId, void *, implementationHandle);

EMERGENCE_CELERITY_EVENT1_DECLARATION (RigidBody2dComponentAddedFixedEvent, UniqueId, objectId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (RigidBody2dComponentAddedCustomToFixedEvent, UniqueId, objectId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (RigidBody2dComponentMassInvalidatedEvent, UniqueId, objectId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (RigidBody2dComponentRemovedEvent, UniqueId, objectId);

EMERGENCE_CELERITY_EVENT5_DECLARATION (CollisionContact2dAddedFixedEvent,
                                       UniqueId,
                                       collisionContactId,
                                       UniqueId,
                                       objectId,
                                       UniqueId,
                                       otherObjectId,
                                       UniqueId,
                                       shapeId,
                                       UniqueId,
                                       otherShapeId);

EMERGENCE_CELERITY_EVENT5_DECLARATION (CollisionContact2dRemovedFixedEvent,
                                       UniqueId,
                                       collisionContactId,
                                       UniqueId,
                                       objectId,
                                       UniqueId,
                                       otherObjectId,
                                       UniqueId,
                                       shapeId,
                                       UniqueId,
                                       otherShapeId);

EMERGENCE_CELERITY_EVENT5_DECLARATION (TriggerContact2dAddedFixedEvent,
                                       UniqueId,
                                       triggerContactId,
                                       UniqueId,
                                       triggerObjectId,
                                       UniqueId,
                                       intruderObjectId,
                                       UniqueId,
                                       triggerShapeId,
                                       UniqueId,
                                       intruderShapeId);

EMERGENCE_CELERITY_EVENT5_DECLARATION (TriggerContact2dRemovedFixedEvent,
                                       UniqueId,
                                       triggerContactId,
                                       UniqueId,
                                       triggerObjectId,
                                       UniqueId,
                                       intruderObjectId,
                                       UniqueId,
                                       triggerShapeId,
                                       UniqueId,
                                       intruderShapeId);
#undef EventsApi

CelerityPhysics2dApi void RegisterPhysicsEvents (EventRegistrar &_registrar) noexcept;
} // namespace Emergence::Celerity
