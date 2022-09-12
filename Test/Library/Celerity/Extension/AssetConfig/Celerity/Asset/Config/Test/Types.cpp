#include <Celerity/Asset/Config/Test/Types.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity::Test
{
const UnitConfig::Reflection &UnitConfig::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (UnitConfig);
        EMERGENCE_MAPPING_REGISTER_REGULAR (id);
        EMERGENCE_MAPPING_REGISTER_REGULAR (health);
        EMERGENCE_MAPPING_REGISTER_REGULAR (armor);
        EMERGENCE_MAPPING_REGISTER_REGULAR (damagePerHit);
        EMERGENCE_MAPPING_REGISTER_REGULAR (hitCastTime);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const BuildingConfig::Reflection &BuildingConfig::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (BuildingConfig);
        EMERGENCE_MAPPING_REGISTER_REGULAR (id);
        EMERGENCE_MAPPING_REGISTER_REGULAR (health);
        EMERGENCE_MAPPING_REGISTER_REGULAR (cost);
        EMERGENCE_MAPPING_REGISTER_REGULAR (buildTime);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity::Test
