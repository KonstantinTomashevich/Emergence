#include <Celerity/UI/UIAssetPin.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const UIAssetPin::Reflection &UIAssetPin::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (UIAssetPin);
        EMERGENCE_MAPPING_REGISTER_REGULAR (assetUserId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (materialId);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
