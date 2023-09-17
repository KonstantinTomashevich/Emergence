#include <Assert/Assert.hpp>

#include <Celerity/Asset/AssetManagerSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const AssetManagerSingleton::Reflection &AssetManagerSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (AssetManagerSingleton);
        EMERGENCE_MAPPING_REGISTER_REGULAR (assetsLeftToLoad);
        EMERGENCE_MAPPING_REGISTER_REGULAR (unusedAssetCount);
        EMERGENCE_MAPPING_REGISTER_REGULAR (automaticallyCleanUnusedAssets);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
