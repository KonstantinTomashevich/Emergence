#include <sstream>
#include <unordered_map>
#include <variant>

#include <Context/Extension/ObjectStorage.hpp>

#include <Handling/Handle.hpp>
#include <Handling/HandleableBase.hpp>

#include <Reference/Test/Tests.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Handling::Test
{
using namespace Context::Extension::Tasks;

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
    Reference::Test::Tasks::Create,
    Move <Handle <HandleableResource>>,
    Copy <Handle <HandleableResource>>,
    MoveAssign <Handle <HandleableResource>>,
    CopyAssign <Handle <HandleableResource>>,
    Delete <Handle <HandleableResource>>,
    Reference::Test::Tasks::CheckStatus>;

std::ostream &operator << (std::ostream &_output, const Reference::Test::Tasks::Create &_task)
{
    return _output << "Construct \"" << _task.name << "\".";
}

std::ostream &operator << (std::ostream &_output, const Move <Handle <HandleableResource>> &_task)
{
    return _output << "Move \"" << _task.sourceName << "\" into \"" << _task.targetName << "\".";
}

std::ostream &operator << (std::ostream &_output, const Copy <Handle <HandleableResource>> &_task)
{
    return _output << "Copy  \"" << _task.sourceName << "\" to \"" << _task.targetName << "\".";
}

std::ostream &operator << (std::ostream &_output, const MoveAssign <Handle <HandleableResource>> &_task)
{
    return _output << "Move \"" << _task.sourceName << "\" into \"" << _task.targetName << "\" using assign operator.";
}

std::ostream &operator << (std::ostream &_output, const CopyAssign <Handle <HandleableResource>> &_task)
{
    return _output << "Assign copy of \"" << _task.sourceName << "\" to \"" << _task.targetName << "\".";
}

std::ostream &operator << (std::ostream &_output, const Delete <Handle <HandleableResource>> &_task)
{
    return _output << "Destruct \"" << _task.name << "\".";
}

std::ostream &operator << (std::ostream &_output, const Reference::Test::Tasks::CheckStatus &_task)
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

struct ExecutionContext final : Context::Extension::ObjectStorage <Handle <HandleableResource>>
{
    bool aliveFlag = false;
    HandleableResource *resource = nullptr;
};

void ExecuteTask (ExecutionContext &_context, const Reference::Test::Tasks::Create &_task)
{
    if (!_context.aliveFlag)
    {
        _context.resource = new HandleableResource (&_context.aliveFlag);
    }

    AddObject (_context, _task.name, _context.resource);
}

void ExecuteTask (ExecutionContext &_context, const Reference::Test::Tasks::CheckStatus &_task)
{
    CHECK_EQUAL (_context.aliveFlag, _task.hasAnyReferences);
}

template <typename T>
std::string ToString (const T &_value)
{
    std::stringstream stream;
    stream << _value;
    return stream.str ();
}

void ExecuteScenario (const std::vector <Task> &_tasks)
{
    ExecutionContext context {};
    LOG (ToString (_tasks));

    for (const Task &wrappedTask : _tasks)
    {
        std::visit (
            [&context] (const auto &_unwrappedTask)
            {
                LOG (ToString (_unwrappedTask));
                ExecuteTask (context, _unwrappedTask);
            },
            wrappedTask);

        for (const auto &pair : context.objects)
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
Task ConvertTask (const Reference::Test::Tasks::Move &_task)
{
    return Move <Handle <HandleableResource>> {_task.sourceName, _task.targetName};
}

template <>
Task ConvertTask (const Reference::Test::Tasks::Copy &_task)
{
    return Copy <Handle <HandleableResource>> {_task.sourceName, _task.targetName};
}

template <>
Task ConvertTask (const Reference::Test::Tasks::MoveAssign &_task)
{
    return MoveAssign <Handle <HandleableResource>> {_task.sourceName, _task.targetName};
}

template <>
Task ConvertTask (const Reference::Test::Tasks::CopyAssign &_task)
{
    return CopyAssign <Handle <HandleableResource>> {_task.sourceName, _task.targetName};
}

template <>
Task ConvertTask (const Reference::Test::Tasks::Delete &_task)
{
    return Delete <Handle <HandleableResource>> {_task.name};
}

void ReferenceTestDriver (const Reference::Test::Scenario &_scenario)
{
    std::vector <Task> tasks;
    for (const Reference::Test::Task &sourceTask : _scenario)
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

REGISTER_ALL_REFERENCE_TESTS (Emergence::Handling::Test::ReferenceTestDriver)

END_SUITE