#include <Assert/Assert.hpp>

#include <Celerity/Pipeline.hpp>

#include <Container/StringBuilder.hpp>

#include <Memory/Profiler/Capture.hpp>

namespace Emergence::Celerity
{
const char *GetPipelineTypeName (PipelineType _type) noexcept
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
        EMERGENCE_ASSERT (false);
        return nullptr;
    }

    EMERGENCE_ASSERT (false);
    return nullptr;
}

static uint32_t GetPipelineTypeColor (PipelineType _type) noexcept
{
    switch (_type)
    {
    case PipelineType::NORMAL:
        return 0xFF009900u;

    case PipelineType::FIXED:
        return 0xFF000099u;

    case PipelineType::CUSTOM:
        return 0xFF990000u;

    case PipelineType::COUNT:
        EMERGENCE_ASSERT (false);
        return 0xFF000001u;
    }

    EMERGENCE_ASSERT (false);
    return 0xFF000001u;
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
    CPU::Profiler::SectionInstance section {executionSection};
    Memory::Profiler::AddMarker (beginMarker);
    executor.Execute ();
    Memory::Profiler::AddMarker (endMarker);
}

Pipeline::Pipeline (Memory::UniqueString _id, PipelineType _type, const Task::Collection &_collection)
    : id (_id),
      type (_type),
      beginMarker (EMERGENCE_BUILD_STRING (id, "Begin")),
      endMarker (EMERGENCE_BUILD_STRING (id, "End")),
      executor (_collection),
      executionSection (*id, GetPipelineTypeColor (type))
{
}
} // namespace Emergence::Celerity
