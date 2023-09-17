#include <CPU/Profiler.hpp>

namespace Emergence::CPU::Profiler
{
void SetThreadName ([[maybe_unused]] const char *_threadName) noexcept
{
}

void MarkFrameEnd () noexcept
{
}

SectionDefinition::SectionDefinition ([[maybe_unused]] const char *_name,
                                      [[maybe_unused]] std::uint32_t _color) noexcept
{
}

SectionDefinition::~SectionDefinition () noexcept = default;

SectionInstance::SectionInstance ([[maybe_unused]] SectionDefinition &_definition) noexcept
{
}

SectionInstance::~SectionInstance () noexcept = default;
} // namespace Emergence::CPU::Profiler
