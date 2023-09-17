#include <Celerity/Render/2d/Sprite2dUvAnimation.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const Sprite2dUvAnimationFrame::Reflection &Sprite2dUvAnimationFrame::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Sprite2dUvAnimationFrame);
        EMERGENCE_MAPPING_REGISTER_REGULAR (uv);
        EMERGENCE_MAPPING_REGISTER_REGULAR (durationNs);
        EMERGENCE_MAPPING_REGISTER_REGULAR (startTimeNs);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const Sprite2dUvAnimation::Reflection &Sprite2dUvAnimation::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Sprite2dUvAnimation);
        EMERGENCE_MAPPING_REGISTER_REGULAR (assetId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (materialInstanceId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (frames);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
