#include <Celerity/Resource/Object/LoadingStateSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
ResourceObjectLoadingSharedState::ResourceObjectLoadingSharedState (
    Resource::Provider::ResourceProvider *_resourceProvider, Resource::Object::TypeManifest _typeManifest) noexcept
    : libraryLoader (_resourceProvider, std::move (_typeManifest))
{
}

const ResourceObjectLoadingStateSingleton::Reflection &ResourceObjectLoadingStateSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (ResourceObjectLoadingStateSingleton);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
