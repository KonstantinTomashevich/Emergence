#include <Celerity/Asset/Test/Data.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity::Test
{
const FirstAssetType::Reflection &FirstAssetType::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (FirstAssetType);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const SecondAssetType::Reflection &SecondAssetType::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (SecondAssetType);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const SingleAssetUser::Reflection &SingleAssetUser::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (SingleAssetUser);
        EMERGENCE_MAPPING_REGISTER_REGULAR (assetUserId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (assetId);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const MultipleAssetUser::Reflection &MultipleAssetUser::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (MultipleAssetUser);
        EMERGENCE_MAPPING_REGISTER_REGULAR (assetUserId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (firstAssetId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (secondAssetId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (thirdAssetId);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

AssetReferenceBindingList GetAssetReferenceBinding () noexcept
{
    AssetReferenceBindingList list {GetAssetBindingAllocationGroup ()};

    AssetReferenceBinding &singleUserBinding = list.emplace_back ();
    singleUserBinding.objectType = SingleAssetUser::Reflect ().mapping;
    singleUserBinding.resourceUserIdField = SingleAssetUser::Reflect ().assetUserId;
    singleUserBinding.references.emplace_back () = {SingleAssetUser::Reflect ().assetId,
                                                    FirstAssetType::Reflect ().mapping};

    AssetReferenceBinding &multipleUserBinding = list.emplace_back ();
    multipleUserBinding.objectType = MultipleAssetUser::Reflect ().mapping;
    multipleUserBinding.resourceUserIdField = MultipleAssetUser::Reflect ().assetUserId;

    multipleUserBinding.references.emplace_back () = {MultipleAssetUser::Reflect ().firstAssetId,
                                                      FirstAssetType::Reflect ().mapping};
    multipleUserBinding.references.emplace_back () = {MultipleAssetUser::Reflect ().secondAssetId,
                                                      SecondAssetType::Reflect ().mapping};
    multipleUserBinding.references.emplace_back () = {MultipleAssetUser::Reflect ().thirdAssetId,
                                                      SecondAssetType::Reflect ().mapping};

    return list;
}
} // namespace Emergence::Celerity::Test
