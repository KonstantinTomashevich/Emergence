#include <sstream>
#include <unordered_map>
#include <variant>

#include <Handling/Handle.hpp>
#include <Handling/HandleableBase.hpp>

#include <Reference/Test/ReferenceStorage.hpp>
#include <Reference/Test/Tests.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Handling::Test
{
using namespace Emergence::Reference::Test::TemplatedTasks;

class HandleableResource final : public HandleableBase
{
public:
    HandleableResource (bool *_aliveFlagPointer)
        : aliveFlagPointer (_aliveFlagPointer)
    {
        REQUIRE (aliveFlagPointer);
        *aliveFlagPointer = true;
    }

    ~HandleableResource () noexcept
    {
        REQUIRE (aliveFlagPointer);
        *aliveFlagPointer = false;
    }

private:
    bool *aliveFlagPointer;
};

using Task = std::variant <
    Emergence::Reference::Test::Tasks::Create,
    Move <Handle <HandleableResource>>,
    Copy <Handle <HandleableResource>>,
    MoveAssign <Handle <HandleableResource>>,
    CopyAssign <Handle <HandleableResource>>,
    Delete <Handle <HandleableResource>>,
    Emergence::Reference::Test::Tasks::CheckStatus>;

std::ostream &operator << (std::ostream &_output, const Emergence::Reference::Test::Tasks::Create &_task)
{
    return _output << "Construct \"" << _task.name << "\".";
}

std::ostream &operator << (std::ostream &_output, const Move <Handle <HandleableResource>> &_task)
{
    return _output << "Move \"" << _task.source << "\" into \"" << _task.target << "\".";
}

std::ostream &operator << (std::ostream &_output, const Copy <Handle <HandleableResource>> &_task)
{
    return _output << "Copy  \"" << _task.source << "\" to \"" << _task.target << "\".";
}

std::ostream &operator << (std::ostream &_output, const MoveAssign <Handle <HandleableResource>> &_task)
{
    return _output << "Move \"" << _task.source << "\" into \"" << _task.target << "\" using assign operator.";
}

std::ostream &operator << (std::ostream &_output, const CopyAssign <Handle <HandleableResource>> &_task)
{
    return _output << "Assign copy of \"" << _task.source << "\" to \"" << _task.target << "\".";
}

std::ostream &operator << (std::ostream &_output, const Emergence::Reference::Test::Tasks::Delete &_task)
{
    return _output << "Destruct \"" << _task.name << "\".";
}

std::ostream &operator << (std::ostream &_output, const Emergence::Reference::Test::Tasks::CheckStatus &_task)
{
    return _output << "Check that resource has any references. Expected result: \"" << _task.hasAnyReferences << "\".";
}

std::ostream &operator << (std::ostream &_output, const std::vector <Task> &_tasks)
{
    _output << "Scenario: " << std::endl;
    for (const Task &wrappedTask : _tasks)
    {
        _output << " - ";
        std::visit (
            [&_output] (const auto &_unwrappedTask)
            {
                _output << _unwrappedTask;
            },
            wrappedTask);

        _output << std::endl;
    }

    return _output;
}

struct ExecutionContext final : Emergence::Reference::Test::ReferenceStorage <Handle <HandleableResource>>
{
    bool aliveFlag = false;
    HandleableResource *resource = nullptr;
};

void ExecuteTask (ExecutionContext &_context, const Emergence::Reference::Test::Tasks::Create &_task)
{
    if (!_context.aliveFlag)
    {
        _context.resource = new HandleableResource (&_context.aliveFlag);
    }

    AddReference (_context, _task.name, _context.resource);
}

void ExecuteTask (ExecutionContext &_context, const Emergence::Reference::Test::Tasks::CheckStatus &_task)
{
    CHECK_EQUAL (_context.aliveFlag, _task.hasAnyReferences);
}

void ExecuteScenario (const std::vector <Task> &_tasks)
{
    ExecutionContext context {};
    LOG ((std::stringstream () << _tasks).str ());

    for (const Task &wrappedTask : _tasks)
    {
        std::visit (
            [&context] (const auto &_unwrappedTask)
            {
                std::stringstream stream;
                stream << _unwrappedTask;
                LOG (stream.str ());
                ExecuteTask (context, _unwrappedTask);
            },
            wrappedTask);

        for (const auto &pair : context.references)
        {
            if (pair.second)
            {
                CHECK_WITH_MESSAGE (
                    context.aliveFlag,
                    "Handle \"", pair.first, "\" is valid. Handles can be valid only when resource is alive.");
                CHECK_EQUAL (pair.second.Get (), context.resource);
            }
        }
    }
}

template <typename SourceTask>
Task ConvertTask (const SourceTask &_task)
{
    return _task;
}

template <>
Task ConvertTask (const Emergence::Reference::Test::Tasks::Move &_task)
{
    return Move <Handle <HandleableResource>> {_task};
}

template <>
Task ConvertTask (const Emergence::Reference::Test::Tasks::Copy &_task)
{
    return Copy <Handle <HandleableResource>> {_task};
}

template <>
Task ConvertTask (const Emergence::Reference::Test::Tasks::MoveAssign &_task)
{
    return MoveAssign <Handle <HandleableResource>> {_task};
}

template <>
Task ConvertTask (const Emergence::Reference::Test::Tasks::CopyAssign &_task)
{
    return CopyAssign <Handle <HandleableResource>> {_task};
}

template <>
Task ConvertTask (const Emergence::Reference::Test::Tasks::Delete &_task)
{
    return Delete <Handle <HandleableResource>> {_task};
}

void ReferenceTestDriver (const Emergence::Reference::Test::Scenario &_scenario)
{
    std::vector <Task> tasks;
    for (const Emergence::Reference::Test::Task &sourceTask : _scenario)
    {
        std::visit (
            [&tasks] (const auto &_task)
            {
                tasks.emplace_back (ConvertTask (_task));
            },
            sourceTask);
    }

    ExecuteScenario (tasks);
}
} // namespace Emergence::Handling::Test

BEGIN_SUITE (HandleManagement)

REGISTER_REFERENCE_TEST (Emergence::Handling::Test::ReferenceTestDriver, ConstructAndDestructMultiple)

REGISTER_REFERENCE_TEST (Emergence::Handling::Test::ReferenceTestDriver, MoveChain)

REGISTER_REFERENCE_TEST (Emergence::Handling::Test::ReferenceTestDriver, MoveCopy)

REGISTER_REFERENCE_TEST (Emergence::Handling::Test::ReferenceTestDriver, CopyMultiple)

REGISTER_REFERENCE_TEST (Emergence::Handling::Test::ReferenceTestDriver, CopyAssignMultiple)

REGISTER_REFERENCE_TEST (Emergence::Handling::Test::ReferenceTestDriver, MoveAssignChain)

END_SUITE