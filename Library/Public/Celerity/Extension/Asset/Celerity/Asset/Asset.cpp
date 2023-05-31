#include <Celerity/Asset/Asset.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const Asset::Reflection &Asset::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Asset);
        EMERGENCE_MAPPING_REGISTER_REGULAR (id);

        static_assert (sizeof (StandardLayout::Mapping) == sizeof (std::uint64_t) ||
                       sizeof (StandardLayout::Mapping) == sizeof (std::uint32_t));

        if constexpr (sizeof (StandardLayout::Mapping) == sizeof (std::uint64_t))
        {
            reflectionData.typeNumber =
                builder.RegisterUInt64 (Memory::UniqueString {"typeStateBlock"}, offsetof (Asset, type));
        }
        else if constexpr (sizeof (StandardLayout::Mapping) == sizeof (std::uint32_t))
        {
            reflectionData.typeNumber =
                builder.RegisterUInt32 (Memory::UniqueString {"typeStateBlock"}, offsetof (Asset, type));
        }

        EMERGENCE_MAPPING_REGISTER_REGULAR (state);
        EMERGENCE_MAPPING_REGISTER_REGULAR (usages);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
