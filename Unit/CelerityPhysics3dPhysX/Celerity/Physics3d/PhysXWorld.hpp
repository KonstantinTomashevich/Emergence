#pragma once

#include <Container/Variant.hpp>

#include <API/Common/MuteWarnings.hpp>

#include <API/Common/Shortcuts.hpp>

#include <Memory/Heap.hpp>

BEGIN_MUTING_WARNINGS
#include <PxFoundation.h>
#include <PxPhysics.h>
#include <PxScene.h>
#include <extensions/PxDefaultAllocator.h>
#include <extensions/PxDefaultCpuDispatcher.h>
#include <extensions/PxDefaultErrorCallback.h>
#include <pvd/PxPvd.h>
END_MUTING_WARNINGS

namespace Emergence::Celerity
{
class ProfiledAllocator final : public physx::PxAllocatorCallback
{
public:
    ProfiledAllocator (Memory::Profiler::AllocationGroup _group);

    void *allocate (std::size_t _size, const char * /*unused*/, const char * /*unused*/, int /*unused*/) override;

    void deallocate (void *_pointer) override;

private:
    Memory::Heap heap;
};

class PhysXJobDispatcher final : public physx::PxCpuDispatcher
{
public:
    void submitTask (physx::PxBaseTask &_task) override;

    [[nodiscard]] std::uint32_t getWorkerCount () const override;
};

struct PhysXWorld final
{
    EMERGENCE_STATIONARY_DATA_TYPE (PhysXWorld);

    Container::Variant<physx::PxDefaultAllocator, ProfiledAllocator> allocator;
    physx::PxDefaultErrorCallback errorCallback;

    physx::PxFoundation *foundation = nullptr;
    physx::PxPhysics *physics = nullptr;

    PhysXJobDispatcher dispatcher;
    physx::PxScene *scene = nullptr;

    bool remoteDebuggerEnabled = false;
    physx::PxPvd *remoteDebugger = nullptr;

    Memory::Heap heap {Memory::Profiler::AllocationGroup {Memory::UniqueString {"PhysX Heap"}}};
};
} // namespace Emergence::Celerity
