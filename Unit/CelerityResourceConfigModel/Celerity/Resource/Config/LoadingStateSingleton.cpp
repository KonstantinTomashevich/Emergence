#include <Celerity/Resource/Config/LoadingStateSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
ResourceConfigLoadingSharedState::ResourceConfigLoadingSharedState (StandardLayout::Mapping _configType) noexcept
    : configType (std::move (_configType)),
      configPool (GetAllocationGroup (), configType.GetObjectSize (), configType.GetObjectAlignment ())
{
}

ResourceConfigLoadingSharedState::~ResourceConfigLoadingSharedState () noexcept
{
    for (void *config : configPool)
    {
        configType.Destruct (config);
    }
}

const ResourceConfigLoadingStateSingleton::Reflection &ResourceConfigLoadingStateSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (ResourceConfigLoadingStateSingleton);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
