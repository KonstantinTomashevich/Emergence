#include <Celerity/Pipeline.hpp>

#include <Container/StringBuilder.hpp>

#include <Memory/Profiler/Capture.hpp>

namespace Emergence::Celerity
{
Memory::UniqueString Pipeline::GetId () const noexcept
{
    return id;
}

PipelineType Pipeline::GetType () const noexcept
{
    return type;
}

void Pipeline::Execute () noexcept
{
    Memory::Profiler::AddMarker (beginMarker);
    executor.Execute ();
    Memory::Profiler::AddMarker (endMarker);
}

Pipeline::Pipeline (Memory::UniqueString _id,
                    PipelineType _type,
                    const Task::Collection &_collection,
                    std::size_t _maximumChildThreads)
    : id (_id),
      type (_type),
      beginMarker (EMERGENCE_BUILD_STRING (id, "Begin")),
      endMarker (EMERGENCE_BUILD_STRING (id, "End")),
      executor (_collection, _maximumChildThreads)
{
}
} // namespace Emergence::Celerity
