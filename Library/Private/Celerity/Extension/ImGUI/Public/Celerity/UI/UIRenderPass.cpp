#include <Celerity/UI/UIRenderPass.hpp>

#include <imgui.h>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
UIRenderPass::UIRenderPass () noexcept = default;

UIRenderPass::~UIRenderPass () noexcept
{
    if (auto *context = static_cast<ImGuiContext *> (nativeContext))
    {
        ImGui::DestroyContext (context);
    }
}

const UIRenderPass::Reflection &UIRenderPass::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (UIRenderPass);
        EMERGENCE_MAPPING_REGISTER_REGULAR (name);
        EMERGENCE_MAPPING_REGISTER_REGULAR (defaultStyleId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (nativeContext);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
