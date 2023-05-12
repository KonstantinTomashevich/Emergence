#include <Resource/Object/Test/Types.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Resource::Object::Test
{
const FirstComponent::Reflection &FirstComponent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (FirstComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (x);
        EMERGENCE_MAPPING_REGISTER_REGULAR (y);
        EMERGENCE_MAPPING_REGISTER_REGULAR (z);
        EMERGENCE_MAPPING_REGISTER_REGULAR (w);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const SecondComponent::Reflection &SecondComponent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (SecondComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (health);
        EMERGENCE_MAPPING_REGISTER_REGULAR (money);
        EMERGENCE_MAPPING_REGISTER_REGULAR (experience);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const MultiComponent::Reflection &MultiComponent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (MultiComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (instanceId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (modelId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (materialId);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const InjectionComponent::Reflection &InjectionComponent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (InjectionComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (injectionId);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const TypeManifest &GetTypeManifest () noexcept
{
    static TypeManifest manifest = [] ()
    {
        TypeManifest typeManifest;
        typeManifest.AddInjection (
            {InjectionComponent::Reflect ().mapping, InjectionComponent::Reflect ().injectionId});

        typeManifest.Register (FirstComponent::Reflect ().mapping, {FirstComponent::Reflect ().objectId});
        typeManifest.Register (SecondComponent::Reflect ().mapping, {SecondComponent::Reflect ().objectId});
        typeManifest.Register (MultiComponent::Reflect ().mapping, {MultiComponent::Reflect ().instanceId});
        typeManifest.Register (InjectionComponent::Reflect ().mapping, {MultiComponent::Reflect ().objectId});
        return typeManifest;
    }();

    return manifest;
}
} // namespace Emergence::Resource::Object::Test
