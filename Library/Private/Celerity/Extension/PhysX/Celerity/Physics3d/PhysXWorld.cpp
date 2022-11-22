#include <Assert/Assert.hpp>

#include <Celerity/Physics3d/PhysXWorld.hpp>

#include <Job/Dispatcher.hpp>

#include <pvd/PxPvdTransport.h>

namespace Emergence::Celerity
{
ProfiledAllocator::ProfiledAllocator (Memory::Profiler::AllocationGroup _group)
    : heap (std::move (_group))
{
}

void *ProfiledAllocator::allocate (size_t _size, const char * /*unused*/, const char * /*unused*/, int /*unused*/)
{
    auto *block = static_cast<uint64_t *> (heap.Acquire (_size + 16u, 16u));
    block[0u] = static_cast<uint64_t> (_size + 16u);
    return block + 2u;
}

void ProfiledAllocator::deallocate (void *_pointer)
{
    auto *block = static_cast<uint64_t *> (_pointer);
    const uint64_t size = *(block - 2u);
    heap.Release (block - 2u, size);
}

void PhysXJobDispatcher::submitTask (physx::PxBaseTask &_task)
{
    Job::Dispatcher::Global ().Dispatch (
        [&_task] ()
        {
            _task.run ();
            _task.release ();
        });
}

uint32_t PhysXJobDispatcher::getWorkerCount () const
{
    return static_cast<uint32_t> (Job::Dispatcher::Global ().GetAvailableThreadsCount ());
}

PhysXWorld::PhysXWorld () noexcept = default;

PhysXWorld::~PhysXWorld () noexcept
{
    if (foundation)
    {
        EMERGENCE_ASSERT (scene);
        EMERGENCE_ASSERT (physics);

        scene->release ();
        physics->release ();

        if (remoteDebugger)
        {
            physx::PxPvdTransport *transport = remoteDebugger->getTransport ();
            remoteDebugger->release ();

            if (transport)
            {
                transport->release ();
            }
        }

        foundation->release ();
    }
}
} // namespace Emergence::Celerity
