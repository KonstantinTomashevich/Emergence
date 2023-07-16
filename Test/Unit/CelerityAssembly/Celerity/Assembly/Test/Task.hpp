#pragma once

#include <SyntaxSugar/MuteWarnings.hpp>

BEGIN_MUTING_PADDING_WARNING

#include <Celerity/PipelineBuilder.hpp>
#include <Celerity/Standard/UniqueId.hpp>

#include <Container/Variant.hpp>
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

    /// \details Sometimes we can not check whole component because of garbage in paddings.
    ///          In this case we set this value to non-zero amount of bytes which should be
    ///          checked in the beginning of the object.
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

using ConfiguratorTask = Container::Variant<Tasks::AddAssemblyDescriptor, Tasks::SpawnPrototype>;

using ValidatorTask = Container::Variant<Tasks::CheckComponent, Tasks::CheckVector3f>;

void AddConfiguratorAndValidator (Emergence::Celerity::PipelineBuilder &_pipelineBuilder,
                                  Container::Vector<ConfiguratorTask> _configuratorTasks,
                                  Container::Vector<ValidatorTask> _validatorTasks);
} // namespace Emergence::Celerity::Test

END_MUTING_WARNINGS
