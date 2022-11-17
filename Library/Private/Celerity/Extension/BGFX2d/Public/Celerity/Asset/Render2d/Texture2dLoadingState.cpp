#include <Celerity/Asset/Render2d/Texture2dLoadingState.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
Texture2dLoadingState::~Texture2dLoadingState () noexcept
{
    if (data)
    {
        EMERGENCE_ASSERT (size > 0u);
        allocator.Release (data, size);
    }

    if (sourceFile)
    {
        fclose (sourceFile);
    }
}

const Texture2dLoadingState::Reflection &Texture2dLoadingState::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Texture2dLoadingState);
        EMERGENCE_MAPPING_REGISTER_REGULAR (assetId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (data);
        EMERGENCE_MAPPING_REGISTER_REGULAR (size);
        EMERGENCE_MAPPING_REGISTER_REGULAR (read);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
