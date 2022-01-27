#include <Celerity/Pipeline.hpp>

#include <Container/StringBuilder.hpp>

#include <Memory/Profiler/Capture.hpp>

namespace Emergence::Celerity
{
Memory::UniqueString Pipeline::GetId () const noexcept
{
    return id;
}

void Pipeline::Execute () noexcept
{
    Memory::Profiler::AddMarker (beginMarker);
    executor.Execute ();
    Memory::Profiler::AddMarker (endMarker);
}

Pipeline::Pipeline (Memory::UniqueString _id, const Task::Collection &_collection, std::size_t _maximumChildThreads)
    : id (_id),
      beginMarker (EMERGENCE_BUILD_STRING (id, "Begin")),
      endMarker (EMERGENCE_BUILD_STRING (id, "End")),
      executor (_collection, _maximumChildThreads)
{
}
} // namespace Emergence::Celerity
