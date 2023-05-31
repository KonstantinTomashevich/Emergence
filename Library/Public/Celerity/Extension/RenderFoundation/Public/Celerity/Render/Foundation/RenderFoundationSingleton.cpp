#include <Celerity/Render/Foundation/MaterialInstance.hpp>
#include <Celerity/Render/Foundation/RenderFoundationSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
Memory::UniqueString RenderFoundationSingleton::GenerateRuntimeMaterialInstanceId (
    Memory::UniqueString _materialInstanceId) const noexcept
{
    EMERGENCE_ASSERT (runtimeMaterialInstanceIdCounter !=
                      std::numeric_limits<decltype (runtimeMaterialInstanceIdCounter)::value_type>::max ());
    const uintptr_t id = const_cast<RenderFoundationSingleton *> (this)->runtimeMaterialInstanceIdCounter++;
    return Memory::UniqueString {
        EMERGENCE_BUILD_STRING (_materialInstanceId, MATERIAL_INSTANCE_RUNTIME_ID_SEPARATOR, id)};
}

const RenderFoundationSingleton::Reflection &RenderFoundationSingleton::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (RenderFoundationSingleton);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
