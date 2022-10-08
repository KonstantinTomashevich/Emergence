#include <Celerity/Assembly/Test/Data.hpp>
#include <Celerity/Transform/TransformComponent.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity::Test
{
const FixedComponentA::Reflection &FixedComponentA::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (FixedComponentA);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (x);
        EMERGENCE_MAPPING_REGISTER_REGULAR (y);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const FixedComponentB::Reflection &FixedComponentB::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (FixedComponentB);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (conditionA);
        EMERGENCE_MAPPING_REGISTER_REGULAR (conditionB);
        EMERGENCE_MAPPING_REGISTER_REGULAR (conditionC);
        EMERGENCE_MAPPING_REGISTER_REGULAR (conditionD);
        EMERGENCE_MAPPING_REGISTER_REGULAR (q);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const FixedMultiComponent::Reflection &FixedMultiComponent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (FixedMultiComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (instanceId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (m);
        EMERGENCE_MAPPING_REGISTER_REGULAR (n);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

uintptr_t FixedMultiComponentIdGeneratorSingleton::GenerateInstanceId () const noexcept
{
    assert (instanceIdCounter != std::numeric_limits<decltype (instanceIdCounter)::value_type>::max ());
    return const_cast<FixedMultiComponentIdGeneratorSingleton *> (this)->instanceIdCounter++;
}

const FixedMultiComponentIdGeneratorSingleton::Reflection &FixedMultiComponentIdGeneratorSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (FixedMultiComponentIdGeneratorSingleton);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const VelocityFixedComponent::Reflection &VelocityFixedComponent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (VelocityFixedComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (globalVelocity);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const NormalVisualComponent::Reflection &NormalVisualComponent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (NormalVisualComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (mainMultiComponentId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (model);
        EMERGENCE_MAPPING_REGISTER_REGULAR (material);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

static UniqueId GenerateInstanceIdForFixedMultiComponent (const void *_argument)
{
    return static_cast<const FixedMultiComponentIdGeneratorSingleton *> (_argument)->GenerateInstanceId ();
}

static constexpr UniqueId MULTI_COMPONENT_INSTANCE_ID_KEY = 0u;

CustomKeyVector GetAssemblerCustomKeys () noexcept
{
    CustomKeyVector customKeys {GetAssemblerConfigurationAllocationGroup ()};
    CustomKeyDescriptor &multiComponentInstanceIdKey = customKeys.emplace_back ();
    multiComponentInstanceIdKey.singletonProviderType = FixedMultiComponentIdGeneratorSingleton::Reflect ().mapping;
    multiComponentInstanceIdKey.providerFunction = GenerateInstanceIdForFixedMultiComponent;
    return customKeys;
}

TypeBindingVector GetFixedAssemblerTypes () noexcept
{
    TypeBindingVector types {GetAssemblerConfigurationAllocationGroup ()};

    TypeDescriptor &transform3dComponent = types.emplace_back ();
    transform3dComponent.type = Transform3dComponent::Reflect ().mapping;
    transform3dComponent.keys.emplace_back () = {Transform3dComponent::Reflect ().objectId,
                                                 ASSEMBLY_OBJECT_ID_KEY_INDEX};
    transform3dComponent.keys.emplace_back () = {Transform3dComponent::Reflect ().parentObjectId,
                                                 ASSEMBLY_OBJECT_ID_KEY_INDEX};

    TypeDescriptor &fixedComponentA = types.emplace_back ();
    fixedComponentA.type = FixedComponentA::Reflect ().mapping;
    fixedComponentA.keys.emplace_back () = {FixedComponentA::Reflect ().objectId, ASSEMBLY_OBJECT_ID_KEY_INDEX};

    TypeDescriptor &fixedComponentB = types.emplace_back ();
    fixedComponentB.type = FixedComponentB::Reflect ().mapping;
    fixedComponentB.keys.emplace_back () = {FixedComponentB::Reflect ().objectId, ASSEMBLY_OBJECT_ID_KEY_INDEX};

    TypeDescriptor &fixedMultiComponent = types.emplace_back ();
    fixedMultiComponent.type = FixedMultiComponent::Reflect ().mapping;
    fixedMultiComponent.keys.emplace_back () = {FixedMultiComponent::Reflect ().objectId, ASSEMBLY_OBJECT_ID_KEY_INDEX};
    fixedMultiComponent.keys.emplace_back () = {FixedMultiComponent::Reflect ().instanceId,
                                                MULTI_COMPONENT_INSTANCE_ID_KEY};

    TypeDescriptor &velocityFixedComponent = types.emplace_back ();
    velocityFixedComponent.type = VelocityFixedComponent::Reflect ().mapping;
    velocityFixedComponent.keys.emplace_back () = {VelocityFixedComponent::Reflect ().objectId,
                                                   ASSEMBLY_OBJECT_ID_KEY_INDEX};
    velocityFixedComponent.rotateVector3fs.emplace_back (VelocityFixedComponent::Reflect ().globalVelocity);

    return types;
}

TypeBindingVector GetNormalAssemblerTypes () noexcept
{
    TypeBindingVector types {GetAssemblerConfigurationAllocationGroup ()};
    TypeDescriptor &normalVisualComponent = types.emplace_back ();
    normalVisualComponent.type = NormalVisualComponent::Reflect ().mapping;
    normalVisualComponent.keys.emplace_back () = {NormalVisualComponent::Reflect ().objectId,
                                                  ASSEMBLY_OBJECT_ID_KEY_INDEX};
    normalVisualComponent.keys.emplace_back () = {NormalVisualComponent::Reflect ().mainMultiComponentId,
                                                  MULTI_COMPONENT_INSTANCE_ID_KEY};
    return types;
}
} // namespace Emergence::Celerity::Test
