#pragma once

#include <CelerityUIModelImGUIApi.hpp>

#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Asset/LoadingSharedState.hpp>
#include <Celerity/Asset/UI/Font.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
class CelerityUIModelImGUIApi FontLoadingSharedState final : public LoadingSharedState<FontLoadingSharedState>
{
public:
    static constexpr const char *ALLOCATION_GROUP_NAME = "FontLoading";

    EMERGENCE_STATIONARY_DATA_TYPE (FontLoadingSharedState);

    FontAsset asset;

    Memory::Heap fontDataHeap {GetHeap ().GetAllocationGroup ()};

    std::uint64_t fontDataSize = 0u;

    std::uint8_t *fontData = nullptr;
};

struct CelerityUIModelImGUIApi FontLoadingState final
{
    Memory::UniqueString assetId;

    Handling::Handle<FontLoadingSharedState> sharedState {new FontLoadingSharedState};

    struct CelerityUIModelImGUIApi Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
