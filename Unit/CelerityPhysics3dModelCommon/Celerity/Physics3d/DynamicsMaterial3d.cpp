#include <Celerity/Physics3d/DynamicsMaterial3d.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
DynamicsMaterial3d::DynamicsMaterial3d () noexcept = default;

DynamicsMaterial3d::DynamicsMaterial3d (DynamicsMaterial3d &&_other) noexcept
    : id (_other.id),
      dynamicFriction (_other.dynamicFriction),
      staticFriction (_other.staticFriction),
      enableFriction (_other.enableFriction),
      restitution (_other.restitution),
      density (_other.density),
      implementationHandle (_other.implementationHandle)
{
    _other.implementationHandle = nullptr;
}

DynamicsMaterial3d::~DynamicsMaterial3d () noexcept = default;

const DynamicsMaterial3d::Reflection &DynamicsMaterial3d::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (DynamicsMaterial3d);
        EMERGENCE_MAPPING_REGISTER_REGULAR (id);
        EMERGENCE_MAPPING_REGISTER_REGULAR (dynamicFriction);
        EMERGENCE_MAPPING_REGISTER_REGULAR (staticFriction);
        EMERGENCE_MAPPING_REGISTER_REGULAR (enableFriction);
        EMERGENCE_MAPPING_REGISTER_REGULAR (restitution);
        EMERGENCE_MAPPING_REGISTER_REGULAR (density);
        EMERGENCE_MAPPING_REGISTER_REGULAR (implementationHandle);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
