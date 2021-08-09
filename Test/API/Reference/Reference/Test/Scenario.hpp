#pragma once

#include <string>
#include <variant>
#include <vector>

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
    std::string source;
    std::string target;
};

struct Copy
{
    std::string source;
    std::string target;
};

struct MoveAssign
{
    std::string source;
    std::string target;
};

struct CopyAssign
{
    std::string source;
    std::string target;
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

using Task = std::variant <
    Tasks::Create,
    Tasks::Move,
    Tasks::Copy,
    Tasks::CopyAssign,
    Tasks::MoveAssign,
    Tasks::Delete,
    Tasks::CheckStatus>;

using Scenario = std::vector <Task>;
} // namespace Emergence::Reference::Test