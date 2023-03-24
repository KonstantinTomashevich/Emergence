#include <Celerity/Physics2d/DynamicsMaterial2d.hpp>

#include <Configuration/ResourceConfigTypeMeta.hpp>

Emergence::Container::Vector<Emergence::Celerity::ResourceConfigTypeMeta> GetResourceConfigTypeMeta () noexcept
{
    return {
        {Emergence::Celerity::DynamicsMaterial2d::Reflect ().mapping,
         Emergence::Celerity::DynamicsMaterial2d::Reflect ().id},
    };
}
