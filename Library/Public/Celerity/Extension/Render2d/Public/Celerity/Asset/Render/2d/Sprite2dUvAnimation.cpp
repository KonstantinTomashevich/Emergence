#include <Celerity/Asset/Render/2d/Sprite2dUvAnimation.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const Sprite2dUvAnimationFrameInfo::Reflection &Sprite2dUvAnimationFrameInfo::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Sprite2dUvAnimationFrameInfo);
        EMERGENCE_MAPPING_REGISTER_REGULAR (uv);
        EMERGENCE_MAPPING_REGISTER_REGULAR (durationS);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const Sprite2dUvAnimationAsset::Reflection &Sprite2dUvAnimationAsset::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Sprite2dUvAnimationAsset);
        EMERGENCE_MAPPING_REGISTER_REGULAR (materialInstanceId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (frames);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
