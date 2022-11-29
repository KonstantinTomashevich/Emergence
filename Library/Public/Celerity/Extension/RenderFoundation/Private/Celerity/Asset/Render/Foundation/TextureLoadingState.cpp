#include <Celerity/Asset/Render/Foundation/TextureLoadingState.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
TextureLoadingState::~TextureLoadingState () noexcept
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

const TextureLoadingState::Reflection &TextureLoadingState::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (TextureLoadingState);
        EMERGENCE_MAPPING_REGISTER_REGULAR (assetId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (data);
        EMERGENCE_MAPPING_REGISTER_REGULAR (size);
        EMERGENCE_MAPPING_REGISTER_REGULAR (read);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
