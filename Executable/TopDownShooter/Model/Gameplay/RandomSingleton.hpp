#pragma once

#include <random>

#include <StandardLayout/Mapping.hpp>

struct RandomSingleton final
{
public:
    float Next () noexcept;

    float NextInRange (float _min, float _max) noexcept;

private:
    std::random_device randomDevice;
    std::mt19937 generator {randomDevice ()};

public:
    struct Reflection
    {
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
