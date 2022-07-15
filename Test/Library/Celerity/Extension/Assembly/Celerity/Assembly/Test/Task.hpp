#pragma once

#if defined(_MSC_VER) && !defined(__clang__)
#    pragma warning(push)
// We do not care about excessive padding in test tasks. Also, we need to do it before variant declaration.
#    pragma warning(disable : 4324)
#endif

#include <variant>

#include <Celerity/PipelineBuilder.hpp>
#include <Celerity/Standard/UniqueId.hpp>

#include <Container/Vector.hpp>

#include <Math/Transform3d.hpp>

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity::Test
{
namespace Tasks
{
struct PatchSource final
{
    StandardLayout::Mapping type;
    const void *changedAfterCreation = nullptr;
};

struct AddAssemblyDescriptor final
{
    Memory::UniqueString descriptorId;
    Container::Vector<PatchSource> patches;
};

struct SpawnPrototype final
{
    Memory::UniqueString descriptorId;
    Math::Transform3d transform {};
    bool logicalTransform = true;
};

struct CheckComponent final
{
    UniqueId id = INVALID_UNIQUE_ID;
    StandardLayout::Mapping type;
    StandardLayout::FieldId idField = 0u;

    const void *expected = nullptr;
    std::size_t comparisonSliceLength = 0u;
};

/// \details We need separate task to check vectors because of floating point precision.
struct CheckVector3f final
{
    UniqueId id = INVALID_UNIQUE_ID;
    StandardLayout::Mapping type;
    StandardLayout::FieldId idField = 0u;

    StandardLayout::FieldId vectorField = 0u;
    Math::Vector3f expected = Math::Vector3f::ZERO;
};
} // namespace Tasks

using ConfiguratorTask = std::variant<Tasks::AddAssemblyDescriptor, Tasks::SpawnPrototype>;

using ValidatorTask = std::variant<Tasks::CheckComponent, Tasks::CheckVector3f>;

void AddConfiguratorAndValidator (Emergence::Celerity::PipelineBuilder &_pipelineBuilder,
                                  Container::Vector<ConfiguratorTask> _configuratorTasks,
                                  Container::Vector<ValidatorTask> _validatorTasks);
} // namespace Emergence::Celerity::Test

#if defined(_MSC_VER) && !defined(__clang__)
#    pragma warning(pop)
#endif
