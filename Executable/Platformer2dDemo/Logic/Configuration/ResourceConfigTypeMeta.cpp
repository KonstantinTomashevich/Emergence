#include <Celerity/Physics2d/DynamicsMaterial2d.hpp>

#include <Configuration/ResourceConfigTypeMeta.hpp>

#include <Platformer/Movement/MovementConfiguration.hpp>

Emergence::Container::Vector<Emergence::Celerity::ResourceConfigTypeMeta> GetResourceConfigTypeMeta () noexcept
{
    return {
        {Emergence::Celerity::DynamicsMaterial2d::Reflect ().mapping,
         Emergence::Celerity::DynamicsMaterial2d::Reflect ().id},
        {MovementConfiguration::Reflect ().mapping, MovementConfiguration::Reflect ().id},
    };
}
