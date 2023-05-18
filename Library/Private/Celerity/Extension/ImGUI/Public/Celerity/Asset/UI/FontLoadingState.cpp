#include <Celerity/Asset/UI/FontLoadingState.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
FontLoadingSharedState::FontLoadingSharedState () noexcept = default;

FontLoadingSharedState::~FontLoadingSharedState () noexcept
{
    if (fontData)
    {
        fontDataHeap.Release (fontData, fontDataSize);
    }
}

const FontLoadingState::Reflection &FontLoadingState::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (FontLoadingState);
        EMERGENCE_MAPPING_REGISTER_REGULAR (assetId);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
