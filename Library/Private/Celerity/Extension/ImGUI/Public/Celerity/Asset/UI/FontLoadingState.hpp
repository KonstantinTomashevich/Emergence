#pragma once

#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Asset/LoadingSharedState.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
class FontLoadingSharedState final : public LoadingSharedState<FontLoadingSharedState>
{
public:
    static constexpr const char *ALLOCATION_GROUP_NAME = "FontLoading";

    EMERGENCE_STATIONARY_DATA_TYPE (FontLoadingSharedState);

    Memory::Heap fontDataHeap {GetHeap().GetAllocationGroup()};

    uint64_t fontDataSize = 0u;

    uint8_t *fontData = nullptr;
};

struct FontLoadingState final
{
    Memory::UniqueString assetId;

    Handling::Handle<FontLoadingSharedState> sharedState {new FontLoadingSharedState};

    struct Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
