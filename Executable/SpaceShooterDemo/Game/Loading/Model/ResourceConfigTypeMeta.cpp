#include <Celerity/Physics/DynamicsMaterial.hpp>

#include <Loading/Model/ResourceConfigTypeMeta.hpp>

Emergence::Container::Vector<Emergence::Celerity::ResourceConfigTypeMeta> PrepareResourceConfigTypeMeta () noexcept
{
    return {
        {Emergence::Celerity::DynamicsMaterial::Reflect ().mapping,
         Emergence::Celerity::DynamicsMaterial::Reflect ().id},
    };
}
