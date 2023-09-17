#pragma once

#include <CelerityResourceConfigModelApi.hpp>

#include <Celerity/Standard/ContextEscape.hpp>

#include <Container/HashSet.hpp>
#include <Container/String.hpp>
#include <Container/Vector.hpp>

#include <Handling/Handle.hpp>
#include <Handling/HandleableBase.hpp>

#include <Memory/OrderedPool.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Describes states of resource config loading routine.
enum class ResourceConfigLoadingState : std::uint8_t
{
    /// \brief Loading routine is executing.
    LOADING,

    /// \brief Loading finished successfully.
    SUCCESSFUL,

    /// \brief Loading finished with errors.
    FAILED,
};

/// \brief Contains resource config library loading state that is shared with background job.
class CelerityResourceConfigModelApi ResourceConfigLoadingSharedState final
    : public ContextEscape<ResourceConfigLoadingSharedState>
{
public:
    static constexpr const char *ALLOCATION_GROUP_NAME = "ResourceConfigLoading";

    ResourceConfigLoadingSharedState (StandardLayout::Mapping _configType) noexcept;

    ResourceConfigLoadingSharedState (const ResourceConfigLoadingSharedState &_other) = delete;

    ResourceConfigLoadingSharedState (ResourceConfigLoadingSharedState &&_other) = delete;

    ~ResourceConfigLoadingSharedState () noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (ResourceConfigLoadingSharedState);

    /// \brief State of this request.
    std::atomic<ResourceConfigLoadingState> loadingState = ResourceConfigLoadingState::LOADING;

    /// \brief Type of configs that are being loaded.
    StandardLayout::Mapping configType;

    /// \brief Pool for temporary allocation of loaded config data.
    Memory::OrderedPool configPool;
};

/// \brief Contains loading states for resource config loading requests.
/// \details Is not designed to be modified from outside.
struct CelerityResourceConfigModelApi ResourceConfigLoadingStateSingleton final
{
    /// \brief States for all active loading requests.
    Container::Vector<Handling::Handle<ResourceConfigLoadingSharedState>> loadingStates {
        Memory::Profiler::AllocationGroup::Top ()};

    struct CelerityResourceConfigModelApi Reflection final
    {
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
