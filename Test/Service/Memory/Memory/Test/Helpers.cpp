#include <Container/String.hpp>

#include <Memory/Test/Helpers.hpp>

namespace Emergence::Memory::Test
{
Profiler::AllocationGroup GetUniqueAllocationGroup () noexcept
{
    static std::atomic_uint counter = 0u;
    return Profiler::AllocationGroup {UniqueString {Container::ToString (counter++)}};
}
} // namespace Emergence::Memory::Test
