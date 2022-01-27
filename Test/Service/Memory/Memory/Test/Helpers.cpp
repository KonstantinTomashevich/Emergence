#include <Container/StringBuilder.hpp>

#include <Memory/Test/Helpers.hpp>

namespace Emergence::Memory::Test
{
Profiler::AllocationGroup GetUniqueAllocationGroup () noexcept
{
    static std::atomic_uint counter = 0u;
    return Profiler::AllocationGroup {UniqueString {EMERGENCE_BUILD_STRING (counter++)}};
}
} // namespace Emergence::Memory::Test
