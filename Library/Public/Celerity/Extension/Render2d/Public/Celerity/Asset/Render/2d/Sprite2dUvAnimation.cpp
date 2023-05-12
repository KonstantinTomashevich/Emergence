#include <Celerity/Asset/Render/2d/Sprite2dUvAnimation.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const Sprite2dUvAnimationAssetHeader::Reflection &Sprite2dUvAnimationAssetHeader::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Sprite2dUvAnimationAssetHeader);
        EMERGENCE_MAPPING_REGISTER_REGULAR (materialInstanceId);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const Sprite2dUvAnimationFrameBundleItem::Reflection &Sprite2dUvAnimationFrameBundleItem::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Sprite2dUvAnimationFrameBundleItem);
        EMERGENCE_MAPPING_REGISTER_REGULAR (uv);
        EMERGENCE_MAPPING_REGISTER_REGULAR (durationS);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
