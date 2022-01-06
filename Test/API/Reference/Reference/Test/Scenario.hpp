#pragma once

#include <string>
#include <variant>

#include <Container/Vector.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

namespace Emergence::Reference::Test
{
namespace Tasks
{
struct Create
{
    std::string name;
};

struct Move
{
    std::string sourceName;
    std::string targetName;
};

struct Copy
{
    std::string sourceName;
    std::string targetName;
};

struct MoveAssign
{
    std::string sourceName;
    std::string targetName;
};

struct CopyAssign
{
    std::string sourceName;
    std::string targetName;
};

struct Delete
{
    std::string name;
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
