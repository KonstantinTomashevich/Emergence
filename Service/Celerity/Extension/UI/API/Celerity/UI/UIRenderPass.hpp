#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct UIRenderPass final
{
    /// \brief Name of the viewport to which this pass is attached.
    Memory::UniqueString name;

    struct Reflection final
    {
        StandardLayout::FieldId name;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
