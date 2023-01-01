#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct UIRenderPass final
{
    EMERGENCE_STATIONARY_DATA_TYPE (UIRenderPass);

    /// \brief Name of the viewport to which this pass is attached.
    Memory::UniqueString name;

    Memory::UniqueString defaultStyleId;

    void *nativeContext = nullptr;

    struct Reflection final
    {
        StandardLayout::FieldId name;
        StandardLayout::FieldId defaultStyleId;
        StandardLayout::FieldId nativeContext;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
