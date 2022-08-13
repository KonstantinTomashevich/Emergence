#pragma once

#include <SyntaxSugar/MuteWarnings.hpp>

#include <Celerity/PipelineBuilder.hpp>

namespace Urho3D
{
class Context;
} // namespace Urho3D

namespace Urho3DUpdate
{
struct Checkpoint final
{
    Checkpoint () = delete;

    static const Emergence::Memory::UniqueString STARTED;
    static const Emergence::Memory::UniqueString FINISHED;
};

void AddToNormalUpdate (Urho3D::Context *_context, Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace Urho3DUpdate
