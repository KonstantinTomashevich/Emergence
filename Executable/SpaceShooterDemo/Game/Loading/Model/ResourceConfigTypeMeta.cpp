#include <Celerity/Physics3d/DynamicsMaterial3d.hpp>

#include <Loading/Model/ResourceConfigTypeMeta.hpp>

Emergence::Container::Vector<Emergence::Celerity::ResourceConfigTypeMeta> PrepareResourceConfigTypeMeta () noexcept
{
    return {
        {Emergence::Celerity::DynamicsMaterial3d::Reflect ().mapping,
         Emergence::Celerity::DynamicsMaterial3d::Reflect ().id},
    };
}
