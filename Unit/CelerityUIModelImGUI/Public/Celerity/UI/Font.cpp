#include <Celerity/UI/Font.hpp>

#include <imgui.h>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
Font::Font () noexcept = default;

Font::~Font () noexcept
{
    if (nativeHandle)
    {
        static_cast<ImFontAtlas *> (nativeHandle)->~ImFontAtlas ();
        ImGui::MemFree (nativeHandle);
    }
}

const Font::Reflection &Font::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Font);
        EMERGENCE_MAPPING_REGISTER_REGULAR (assetId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (nativeHandle);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
