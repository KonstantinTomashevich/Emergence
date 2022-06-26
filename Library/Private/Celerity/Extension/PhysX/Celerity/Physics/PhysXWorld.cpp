#include <cassert>

#include <Celerity/Physics/PhysXWorld.hpp>

#include <pvd/PxPvdTransport.h>

namespace Emergence::Celerity
{
ProfiledAllocator::ProfiledAllocator (Memory::Profiler::AllocationGroup _group) : heap (std::move (_group))
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

PhysXWorld::PhysXWorld () noexcept = default;

PhysXWorld::~PhysXWorld () noexcept
{
    if (foundation)
    {
        assert (scene);
        assert (dispatcher);
        assert (physics);

        scene->release ();
        dispatcher->release ();
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
