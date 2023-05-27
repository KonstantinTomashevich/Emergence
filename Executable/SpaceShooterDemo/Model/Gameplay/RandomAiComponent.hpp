#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <Math/Vector3f.hpp>

#include <StandardLayout/Mapping.hpp>

struct RandomAiComponent final
{
    Emergence::Celerity::UniqueId objectId = Emergence::Celerity::INVALID_UNIQUE_ID;

    Emergence::Math::Vector3f currentTargetPoint {0.0f, 0.0f, 0.0f};

    std::uint64_t chaseTargetPointUntilNs = 0u;

    float averageShotsPerS = 1.0f;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId objectId;
        Emergence::StandardLayout::FieldId currentTargetPoint;
        Emergence::StandardLayout::FieldId chaseTargetPointUntilNs;
        Emergence::StandardLayout::FieldId averageShotsPerS;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
