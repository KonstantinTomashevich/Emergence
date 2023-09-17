#pragma once

#include <CelerityInputLogicApi.hpp>

#include <Celerity/PipelineBuilder.hpp>

#include <InputStorage/FrameInputAccumulator.hpp>

namespace Emergence::Celerity::Input
{
/// \brief Checkpoints supported by tasks from ::AddToFixedUpdate and ::AddToNormalUpdate.
struct CelerityInputLogicApi Checkpoint final
{
    Checkpoint () = delete;

    /// \brief After this checkpoint action dispatch from InputActionHolder's is started.
    static const Memory::UniqueString ACTION_DISPATCH_STARTED;

    /// \brief Action dispatch finishes before this checkpoint
    ///        and external features can insert custom InputActionComponent's.
    static const Memory::UniqueString CUSTOM_ACTION_COMPONENT_INSERT_ALLOWED;

    /// \brief Insertion of custom InputActionComponent's finishes before this checkpoint
    ///        and InputActionComponent data can be read by end users.
    static const Memory::UniqueString ACTION_COMPONENT_READ_ALLOWED;
};

/// \brief Adds fixed update tasks that update InputActionComponent's and dispatch fixed pipeline actions.
CelerityInputLogicApi void AddToFixedUpdate (PipelineBuilder &_builder) noexcept;

/// \brief Adds normal update tasks that update InputActionComponent's,
///        update input triggers and dispatch normal pipeline actions.
CelerityInputLogicApi void AddToNormalUpdate (PipelineBuilder &_builder,
                                              InputStorage::FrameInputAccumulator *_inputAccumulator) noexcept;
} // namespace Emergence::Celerity::Input
