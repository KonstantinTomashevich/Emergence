#pragma once

#include <Celerity/Locale/LocaleConfiguration.hpp>

#include <Handling/Handle.hpp>
#include <Handling/HandleableBase.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief States of locale loading for LocaleLoadingSharedState synchronization.
enum class LocaleLoadingState : std::uint8_t
{
    /// \brief Loading routine is executing.
    LOADING,

    /// \brief Loading finished successfully.
    SUCCESSFUL,

    /// \brief Loading finished with errors.
    FAILED,
};

/// \brief Contains locale loading state that is shared with background job.
class LocaleLoadingSharedState final : public Handling::HandleableBase
{
public:
    void *operator new (std::size_t /*unused*/) noexcept;

    void operator delete (void *_pointer) noexcept;

    /// \brief Configuration that is currently being loaded from resources.
    LocaleConfiguration configurationInLoading;

    /// \brief Describes in which state locale loading is right now.
    std::atomic<LocaleLoadingState> loadingState;

private:
    /// \brief Allocation group used by ::GetHeap.
    static Memory::Profiler::AllocationGroup GetAllocationGroup () noexcept;

    /// \brief Heap for allocating instances of this state.
    static Memory::Heap &GetHeap () noexcept;
};

/// \brief Stores global configuration and state of localization routine.
struct LocaleSingleton final
{
    /// \brief Target locale to be loaded and applied. Edit this field if you need to change locale.
    Memory::UniqueString targetLocale;

    /// \brief Locale that is currently loaded. Designed to be readonly for users.
    Memory::UniqueString loadedLocale;

    /// \brief Locale that is currently being loaded if any. Designed to be readonly for users.
    Memory::UniqueString loadingLocale;

    /// \brief Shared state for background loading through job dispatcher.
    Handling::Handle<LocaleLoadingSharedState> sharedState {new LocaleLoadingSharedState};

    struct Reflection final
    {
        StandardLayout::FieldId targetLocale;
        StandardLayout::FieldId loadedLocale;
        StandardLayout::FieldId loadingLocale;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
