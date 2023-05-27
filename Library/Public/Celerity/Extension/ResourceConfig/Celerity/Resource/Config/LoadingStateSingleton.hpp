#pragma once

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
class ResourceConfigLoadingSharedState final : public Handling::HandleableBase
{
public:
    void *operator new (std::size_t /*unused*/) noexcept;

    void operator delete (void *_pointer) noexcept;

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

private:
    /// \brief Allocation group used by ::GetHeap.
    static Memory::Profiler::AllocationGroup GetAllocationGroup () noexcept;

    /// \brief Heap for allocating instances of this state.
    static Memory::Heap &GetHeap () noexcept;
};

/// \brief Contains loading states for resource config loading requests.
/// \details Is not designed to be modified from outside.
struct ResourceConfigLoadingStateSingleton final
{
    /// \brief States for all active loading requests.
    Container::Vector<Handling::Handle<ResourceConfigLoadingSharedState>> loadingStates {
        Memory::Profiler::AllocationGroup::Top ()};

    struct Reflection final
    {
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
