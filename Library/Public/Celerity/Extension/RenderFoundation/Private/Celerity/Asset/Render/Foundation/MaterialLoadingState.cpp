#include <Celerity/Asset/Render/Foundation/MaterialLoadingState.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
MaterialLoadingSharedState::MaterialLoadingSharedState () noexcept = default;

MaterialLoadingSharedState::~MaterialLoadingSharedState () noexcept
{
    if (vertexShaderData)
    {
        shaderDataHeap.Release (vertexShaderData, vertexSharedSize);
    }

    if (fragmentShaderData)
    {
        shaderDataHeap.Release (fragmentShaderData, fragmentSharedSize);
    }
}

const MaterialLoadingState::Reflection &MaterialLoadingState::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (MaterialLoadingState);
        EMERGENCE_MAPPING_REGISTER_REGULAR (assetId);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
