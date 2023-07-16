#pragma once

#include <cstdint>

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Shortcuts.hpp>

namespace Emergence::CPU::Profiler
{
/// \brief Marks current thread with given readable name for profiling.
void SetThreadName (const char *_threadName) noexcept;

/// \brief Marks frame end for frame-based applications.
void MarkFrameEnd () noexcept;

/// \brief Stores persistent information about section of code that can be profiled.
class SectionDefinition final
{
public:
    /// \brief Creates section with given name and given visualization color.
    SectionDefinition (const char *_name, std::uint32_t _color) noexcept;

    SectionDefinition (const SectionDefinition &_other) = delete;

    SectionDefinition (SectionDefinition &&_other) noexcept = delete;

    // NOLINTNEXTLINE(performance-trivially-destructible): Trivially destructible only for None implementation.
    ~SectionDefinition () noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (SectionDefinition);

private:
    friend class SectionInstance;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t) * 4u);
};

/// \brief Instantiates given section and marks it as currently running until this object is destroyed.
/// \details Sections use stack hierarchy, therefore running sections inside sections results in flame graph.
///          Keep in mind that parent-child relations are created on per-instance basis, not per-definition.
class SectionInstance final
{
public:
    SectionInstance (SectionDefinition &_definition) noexcept;

    SectionInstance (const SectionInstance &_other) = delete;

    SectionInstance (SectionInstance &&_other) noexcept = delete;

    // NOLINTNEXTLINE(performance-trivially-destructible): Trivially destructible only for None implementation.
    ~SectionInstance () noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (SectionInstance);

private:
    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t) * 2u);
};
} // namespace Emergence::CPU::Profiler
