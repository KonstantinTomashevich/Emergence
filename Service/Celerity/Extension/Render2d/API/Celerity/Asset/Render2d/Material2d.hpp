#pragma once

#include <Celerity/Render2d/Material2d.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct Material2dAssetHeader final
{
    Memory::UniqueString vertexShader;
    Memory::UniqueString fragmentShader;

    struct Reflection final
    {
        StandardLayout::FieldId vertexShader;
        StandardLayout::FieldId fragmentShader;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct Uniform2dBundleItem final
{
    Memory::UniqueString name;
    Uniform2dType type = Uniform2dType::VECTOR_4F;

    struct Reflection final
    {
        StandardLayout::FieldId name;
        StandardLayout::FieldId type;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
