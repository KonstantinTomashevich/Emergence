#include <CPU/Profiler.hpp>

#include <SyntaxSugar/BlockCast.hpp>

#include <tracy/Tracy.hpp>

namespace Emergence::CPU::Profiler
{
void SetThreadName ([[maybe_unused]] const char *_threadName) noexcept
{
    tracy::SetThreadName (_threadName);
}

void MarkFrameEnd () noexcept
{
    tracy::Profiler::SendFrameMark (nullptr);
}

SectionDefinition::SectionDefinition ([[maybe_unused]] const char *_name, std::uint32_t _color) noexcept
{
    auto &tracyData = block_cast<tracy::SourceLocationData> (data);
    tracyData.name = _name;
    tracyData.file = nullptr;
    tracyData.function = nullptr;
    tracyData.line = 0u;
    tracyData.color = _color;
}

SectionDefinition::~SectionDefinition () noexcept
{
    block_cast<tracy::SourceLocationData> (data).~SourceLocationData ();
}

SectionInstance::SectionInstance (SectionDefinition &_definition) noexcept
{
    new (data.data ()) tracy::ScopedZone (&block_cast<tracy::SourceLocationData> (_definition.data));
}

SectionInstance::~SectionInstance () noexcept
{
    block_cast<tracy::ScopedZone> (data).~ScopedZone ();
}
} // namespace Emergence::CPU::Profiler
