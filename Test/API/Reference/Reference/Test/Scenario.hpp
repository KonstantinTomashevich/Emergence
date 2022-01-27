#pragma once

#include <variant>

#include <Container/String.hpp>

#include <Container/Vector.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

namespace Emergence::Reference::Test
{
namespace Tasks
{
struct Create
{
    Container::String name;
};

struct Move
{
    Container::String sourceName;
    Container::String targetName;
};

struct Copy
{
    Container::String sourceName;
    Container::String targetName;
};

struct MoveAssign
{
    Container::String sourceName;
    Container::String targetName;
};

struct CopyAssign
{
    Container::String sourceName;
    Container::String targetName;
};

struct Delete
{
    Container::String name;
};

struct CheckStatus
{
    bool hasAnyReferences = false;
};
} // namespace Tasks

using Task = std::variant<Tasks::Create,
                          Tasks::Move,
                          Tasks::Copy,
                          Tasks::CopyAssign,
                          Tasks::MoveAssign,
                          Tasks::Delete,
                          Tasks::CheckStatus>;

using Scenario = Container::Vector<Task>;
} // namespace Emergence::Reference::Test
