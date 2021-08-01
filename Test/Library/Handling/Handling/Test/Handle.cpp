#include <sstream>
#include <unordered_map>
#include <variant>

#include <Handling/Handle.hpp>
#include <Handling/HandleableBase.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Handling::Test
{
class HandleableSingletonResource final : public HandleableBase
{
public:
    ~HandleableSingletonResource () noexcept
    {
        instance = nullptr;
    }

    static HandleableSingletonResource *AcquireInstance () noexcept
    {
        if (!instance)
        {
            instance = new HandleableSingletonResource ();
        }

        return instance;
    }

    static bool HasInstance () noexcept
    {
        return instance;
    }

    static void Cleanup () noexcept
    {
        delete instance;
        instance = nullptr;
    }

private:
    HandleableSingletonResource () noexcept
    {
        instance = this;
    }

    static HandleableSingletonResource *instance;
};

HandleableSingletonResource *HandleableSingletonResource::instance = nullptr;

struct ConstructHandle
{
    std::string name;
};

struct DestructHandle
{
    std::string name;
};

struct CopyHandle
{
    std::string sourceName;
    std::string targetName;
};

struct MoveHandle
{
    std::string sourceName;
    std::string targetName;
};

struct CopyAssignHandle
{
    std::string sourceName;
    std::string targetName;
};

struct MoveAssignHandle
{
    std::string sourceName;
    std::string targetName;
};

using Task = std::variant <
    ConstructHandle,
    DestructHandle,
    CopyHandle,
    MoveHandle,
    CopyAssignHandle,
    MoveAssignHandle>;

std::string ToString (const ConstructHandle &_task)
{
    return "Construct \"" + _task.name + "\".";
}

std::string ToString (const DestructHandle &_task)
{
    return "Destruct \"" + _task.name + "\".";
}

std::string ToString (const CopyHandle &_task)
{
    return "Copy  \"" + _task.sourceName + "\" to \"" + _task.targetName + "\".";
}

std::string ToString (const MoveHandle &_task)
{
    return "Move \"" + _task.sourceName + "\" into \"" + _task.targetName + "\".";
}

std::string ToString (const CopyAssignHandle &_task)
{
    return "Assign copy of  \"" + _task.sourceName + "\" to \"" + _task.targetName + "\".";
}

std::string ToString (const MoveAssignHandle &_task)
{
    return "Move \"" + _task.sourceName + "\" into \"" + _task.targetName + "\" using assign operator.";
}

class Scenario final
{
public:
    explicit Scenario (std::vector <Task> _tasks);

private:
    std::string GetFullDescription () const;

    void ExecuteTask (const ConstructHandle &_task);

    void ExecuteTask (const DestructHandle &_task);

    void ExecuteTask (const CopyHandle &_task);

    void ExecuteTask (const MoveHandle &_task);

    void ExecuteTask (const CopyAssignHandle &_task);

    void ExecuteTask (const MoveAssignHandle &_task);

private:
    std::vector <Task> tasks;
    std::unordered_map <std::string, Handle <HandleableSingletonResource>> handles;
};

Scenario::Scenario (std::vector <Task> _tasks)
    : tasks (std::move (_tasks)),
      handles ()
{
    LOG (GetFullDescription ());
    for (const Task &packedTask : tasks)
    {
        std::visit (
            [this] (const auto &task)
            {
                LOG (ToString (task));
                ExecuteTask (task);
            },
            packedTask);
    }

    handles.clear ();
    CHECK (HandleableSingletonResource::HasInstance () == false);
    HandleableSingletonResource::Cleanup ();
}

std::string Scenario::GetFullDescription () const
{
    std::stringstream output;
    output << "Scenario: " << std::endl;
    for (const Task &packedTask : tasks)
    {
        std::visit (
            [&output] (const auto &task)
            {
                output << " - " << ToString (task) << std::endl;
            },
            packedTask);
    }

    return output.str ();
}

void Scenario::ExecuteTask (const ConstructHandle &_task)
{
    REQUIRE (handles.find (_task.name) == handles.end ());
    auto[iterator, emplaceResult] = handles.emplace (
        _task.name, HandleableSingletonResource::AcquireInstance ());

    REQUIRE (emplaceResult);
    CHECK (iterator->second);
}

void Scenario::ExecuteTask (const DestructHandle &_task)
{
    auto iterator = handles.find (_task.name);
    REQUIRE (iterator != handles.end ());
    handles.erase (iterator);
}

void Scenario::ExecuteTask (const CopyHandle &_task)
{
    auto sourceIterator = handles.find (_task.sourceName);
    REQUIRE (sourceIterator != handles.end ());
    REQUIRE (handles.find (_task.targetName) == handles.end ());

    auto[iterator, emplaceResult] = handles.emplace (_task.targetName, sourceIterator->second);
    REQUIRE (emplaceResult);
    CHECK (iterator->second == sourceIterator->second);
}

void Scenario::ExecuteTask (const MoveHandle &_task)
{
    auto sourceIterator = handles.find (_task.sourceName);
    REQUIRE (sourceIterator != handles.end ());
    REQUIRE (handles.find (_task.targetName) == handles.end ());

    bool sourceValid {sourceIterator->second};
    auto[iterator, emplaceResult] = handles.emplace (
        _task.targetName, std::move (sourceIterator->second));

    REQUIRE (emplaceResult);
    CHECK (!sourceIterator->second);
    CHECK (bool {iterator->second} == sourceValid);
}

void Scenario::ExecuteTask (const CopyAssignHandle &_task)
{
    auto sourceIterator = handles.find (_task.sourceName);
    REQUIRE (sourceIterator != handles.end ());

    auto targetIterator = handles.find (_task.targetName);
    REQUIRE (targetIterator != handles.end ());

    bool sourceValid {sourceIterator->second};
    targetIterator->second = sourceIterator->second;
    CHECK (sourceIterator->second == targetIterator->second);

    if (sourceIterator == targetIterator)
    {
        HandleableSingletonResource *expected =
            sourceValid ? HandleableSingletonResource::AcquireInstance () : nullptr;
        CHECK (sourceIterator->second.Get () == expected);
    }
}

void Scenario::ExecuteTask (const MoveAssignHandle &_task)
{
    auto sourceIterator = handles.find (_task.sourceName);
    REQUIRE (sourceIterator != handles.end ());

    auto targetIterator = handles.find (_task.targetName);
    REQUIRE (targetIterator != handles.end ());

    bool sourceValid {sourceIterator->second};
    targetIterator->second = std::move (sourceIterator->second);

    if (sourceIterator == targetIterator)
    {
        HandleableSingletonResource *expected =
            sourceValid ? HandleableSingletonResource::AcquireInstance () : nullptr;
        CHECK (sourceIterator->second.Get () == expected);
    }
    else
    {
        CHECK (!sourceIterator->second);
        CHECK (bool {targetIterator->second} == sourceValid);
    }
}
} // namespace Emergence::Handling::Test

BEGIN_SUITE (HandleManagement)

/// It's difficult to describe all handle manipulations with short names and there
/// is not real sense to do it, because scenarios describe these manipulations better.
/// Therefore we use line numbers to generate unique test case names.

TEST_CASE (__LINE__)
{
    Emergence::Handling::Test::Scenario (
        {
            Emergence::Handling::Test::ConstructHandle {"first"},
        });
}

TEST_CASE (__LINE__)
{
    Emergence::Handling::Test::Scenario (
        {
            Emergence::Handling::Test::ConstructHandle {"first"},
            Emergence::Handling::Test::DestructHandle {"first"},
        });
}

TEST_CASE (__LINE__)
{
    Emergence::Handling::Test::Scenario (
        {
            Emergence::Handling::Test::ConstructHandle {"first"},
            Emergence::Handling::Test::ConstructHandle {"second"},
            Emergence::Handling::Test::DestructHandle {"first"},
            Emergence::Handling::Test::DestructHandle {"second"},
        });
}

TEST_CASE (__LINE__)
{
    Emergence::Handling::Test::Scenario (
        {
            Emergence::Handling::Test::ConstructHandle {"first"},
            Emergence::Handling::Test::ConstructHandle {"second"},
            Emergence::Handling::Test::DestructHandle {"second"},
        });
}

TEST_CASE (__LINE__)
{
    Emergence::Handling::Test::Scenario (
        {
            Emergence::Handling::Test::ConstructHandle {"first"},
            Emergence::Handling::Test::CopyHandle {"first", "second"},
            Emergence::Handling::Test::DestructHandle {"first"},
        });
}

TEST_CASE (__LINE__)
{
    Emergence::Handling::Test::Scenario (
        {
            Emergence::Handling::Test::ConstructHandle {"first"},
            Emergence::Handling::Test::CopyHandle {"first", "second"},
            Emergence::Handling::Test::DestructHandle {"second"},
        });
}

TEST_CASE (__LINE__)
{
    Emergence::Handling::Test::Scenario (
        {
            Emergence::Handling::Test::ConstructHandle {"first"},
            Emergence::Handling::Test::MoveHandle {"first", "second"},
            Emergence::Handling::Test::DestructHandle {"first"},
        });
}

TEST_CASE (__LINE__)
{
    Emergence::Handling::Test::Scenario (
        {
            Emergence::Handling::Test::ConstructHandle {"first"},
            Emergence::Handling::Test::MoveHandle {"first", "second"},
            Emergence::Handling::Test::DestructHandle {"second"},
        });
}

TEST_CASE (__LINE__)
{
    Emergence::Handling::Test::Scenario (
        {
            Emergence::Handling::Test::ConstructHandle {"first"},
            Emergence::Handling::Test::MoveHandle {"first", "second"},
            Emergence::Handling::Test::ConstructHandle {"third"},
            Emergence::Handling::Test::DestructHandle {"second"},
        });
}

TEST_CASE (__LINE__)
{
    Emergence::Handling::Test::Scenario (
        {
            Emergence::Handling::Test::ConstructHandle {"first"},
            Emergence::Handling::Test::MoveHandle {"first", "second"},
            Emergence::Handling::Test::MoveHandle {"first", "third"},
            Emergence::Handling::Test::DestructHandle {"second"},
        });
}

TEST_CASE (__LINE__)
{
    Emergence::Handling::Test::Scenario (
        {
            Emergence::Handling::Test::ConstructHandle {"first"},
            Emergence::Handling::Test::MoveHandle {"first", "second"},
            Emergence::Handling::Test::CopyHandle {"first", "third"},
            Emergence::Handling::Test::DestructHandle {"second"},
        });
}

TEST_CASE (__LINE__)
{
    Emergence::Handling::Test::Scenario (
        {
            Emergence::Handling::Test::ConstructHandle {"first"},
            Emergence::Handling::Test::CopyAssignHandle {"first", "first"},
        });
}

TEST_CASE (__LINE__)
{
    Emergence::Handling::Test::Scenario (
        {
            Emergence::Handling::Test::ConstructHandle {"first"},
            Emergence::Handling::Test::MoveAssignHandle {"first", "first"},
        });
}

TEST_CASE (__LINE__)
{
    Emergence::Handling::Test::Scenario (
        {
            Emergence::Handling::Test::ConstructHandle {"first"},
            Emergence::Handling::Test::ConstructHandle {"second"},
            Emergence::Handling::Test::CopyAssignHandle {"first", "second"},
            Emergence::Handling::Test::DestructHandle {"first"},
        });
}

TEST_CASE (__LINE__)
{
    Emergence::Handling::Test::Scenario (
        {
            Emergence::Handling::Test::ConstructHandle {"first"},
            Emergence::Handling::Test::ConstructHandle {"second"},
            Emergence::Handling::Test::MoveAssignHandle {"first", "second"},
            Emergence::Handling::Test::DestructHandle {"first"},
        });
}

TEST_CASE (__LINE__)
{
    Emergence::Handling::Test::Scenario (
        {
            Emergence::Handling::Test::ConstructHandle {"first"},
            Emergence::Handling::Test::ConstructHandle {"second"},
            Emergence::Handling::Test::MoveAssignHandle {"first", "second"},
            Emergence::Handling::Test::DestructHandle {"second"},
        });
}

TEST_CASE (__LINE__)
{
    Emergence::Handling::Test::Scenario (
        {
            Emergence::Handling::Test::ConstructHandle {"first"},
            Emergence::Handling::Test::ConstructHandle {"second"},
            Emergence::Handling::Test::ConstructHandle {"third"},
            Emergence::Handling::Test::MoveAssignHandle {"first", "second"},
            Emergence::Handling::Test::MoveAssignHandle {"first", "third"},
            Emergence::Handling::Test::DestructHandle {"second"},
        });
}

TEST_CASE (__LINE__)
{
    Emergence::Handling::Test::Scenario (
        {
            Emergence::Handling::Test::ConstructHandle {"first"},
            Emergence::Handling::Test::ConstructHandle {"second"},
            Emergence::Handling::Test::ConstructHandle {"third"},
            Emergence::Handling::Test::MoveAssignHandle {"first", "second"},
            Emergence::Handling::Test::CopyAssignHandle {"first", "third"},
            Emergence::Handling::Test::DestructHandle {"second"},
        });
}

END_SUITE