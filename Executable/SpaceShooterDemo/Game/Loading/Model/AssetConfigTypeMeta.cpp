#include <Celerity/Physics/DynamicsMaterial.hpp>

#include <Loading/Model/AssetConfigTypeMeta.hpp>

Emergence::Container::Vector<Emergence::Celerity::AssetConfigTypeMeta> PrepareAssetConfigTypeMeta () noexcept
{
    return {
        {Emergence::Celerity::DynamicsMaterial::Reflect ().mapping,
         Emergence::Celerity::DynamicsMaterial::Reflect ().id},
    };
}
