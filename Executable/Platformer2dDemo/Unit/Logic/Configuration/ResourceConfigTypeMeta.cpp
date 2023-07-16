#include <Celerity/Physics2d/DynamicsMaterial2d.hpp>

#include <Configuration/ResourceConfigTypeMeta.hpp>

#include <Platformer/Animation/CharacterAnimationConfiguration.hpp>
#include <Platformer/Movement/MovementConfiguration.hpp>

Emergence::Container::Vector<Emergence::Celerity::ResourceConfigTypeMeta> GetResourceConfigTypeMeta () noexcept
{
    return {
        {CharacterAnimationConfiguration::Reflect ().mapping, CharacterAnimationConfiguration::Reflect ().id},
        {Emergence::Celerity::DynamicsMaterial2d::Reflect ().mapping,
         Emergence::Celerity::DynamicsMaterial2d::Reflect ().id},
        {MovementConfiguration::Reflect ().mapping, MovementConfiguration::Reflect ().id},
    };
}
