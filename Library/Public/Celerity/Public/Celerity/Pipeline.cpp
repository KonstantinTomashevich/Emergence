#include <cassert>

#include <Celerity/Pipeline.hpp>

#include <Container/StringBuilder.hpp>

#include <Memory/Profiler/Capture.hpp>

namespace Emergence::Celerity
{
const char *GetPipelineTypeName (PipelineType _type)
{
    switch (_type)
    {
    case PipelineType::NORMAL:
        return "Normal";

    case PipelineType::FIXED:
        return "Fixed";

    case PipelineType::CUSTOM:
        return "Custom";

    case PipelineType::COUNT:
        assert (false);
        return nullptr;
    }

    assert (false);
    return nullptr;
}

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

Pipeline::Pipeline (Memory::UniqueString _id, PipelineType _type, const Task::Collection &_collection)
    : id (_id),
      type (_type),
      beginMarker (EMERGENCE_BUILD_STRING (id, "Begin")),
      endMarker (EMERGENCE_BUILD_STRING (id, "End")),
      executor (_collection)
{
}
} // namespace Emergence::Celerity
