#include <iostream>
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

class Seed final
{
public:
    explicit Seed (std::vector <Task> _tasks)
        : tasks (std::move (_tasks))
    {
    }

    void GrowAndTest () const
    {
        std::unordered_map <std::string, Handle <HandleableSingletonResource>> handles;
        for (const Task &task : tasks)
        {
            std::visit (
                [&handles] (const auto &unwrappedTask)
                {
                    using CurrentTask = std::decay_t <decltype (unwrappedTask)>;
                    if constexpr (std::is_same_v <CurrentTask, ConstructHandle>)
                    {
                        const auto &constructTask = static_cast <const ConstructHandle &> (unwrappedTask);
                        INFO ("Constructing handle \"", constructTask.name, "\" from raw instance.");

                        REQUIRE (handles.find (constructTask.name) == handles.end ());
                        auto[iterator, emplaceResult] = handles.emplace (
                            constructTask.name, HandleableSingletonResource::AcquireInstance ());

                        REQUIRE (emplaceResult);
                        CHECK (iterator->second);
                    }
                    else if constexpr (std::is_same_v <CurrentTask, DestructHandle>)
                    {
                        const auto &destructTask = static_cast <const DestructHandle &> (unwrappedTask);
                        INFO ("Destructing handle \"", destructTask.name, "\".");

                        auto iterator = handles.find (destructTask.name);
                        REQUIRE (iterator != handles.end ());
                        handles.erase (iterator);
                    }
                    else if constexpr (std::is_same_v <CurrentTask, CopyHandle>)
                    {
                        const auto &copyTask = static_cast <const CopyHandle &> (unwrappedTask);
                        INFO ("Copying handle \"", copyTask.sourceName, "\" to \"", copyTask.targetName, "\".");

                        auto sourceIterator = handles.find (copyTask.sourceName);
                        REQUIRE (sourceIterator != handles.end ());
                        REQUIRE (handles.find (copyTask.targetName) == handles.end ());

                        auto[iterator, emplaceResult] = handles.emplace (copyTask.targetName, sourceIterator->second);
                        REQUIRE (emplaceResult);
                        CHECK (iterator->second == sourceIterator->second);
                    }
                    else if constexpr (std::is_same_v <CurrentTask, MoveHandle>)
                    {
                        const auto &moveTask = static_cast <const MoveHandle &> (unwrappedTask);
                        INFO ("Moving handle \"", moveTask.sourceName, "\" into \"", moveTask.targetName, "\".");

                        auto sourceIterator = handles.find (moveTask.sourceName);
                        REQUIRE (sourceIterator != handles.end ());
                        REQUIRE (handles.find (moveTask.targetName) == handles.end ());

                        bool sourceValid = sourceIterator->second;
                        auto[iterator, emplaceResult] = handles.emplace (
                            moveTask.targetName, std::move (sourceIterator->second));

                        REQUIRE (emplaceResult);
                        CHECK (!sourceIterator->second);
                        CHECK (iterator->second == sourceValid);
                    }
                    else if constexpr (std::is_same_v <CurrentTask, CopyAssignHandle>)
                    {
                        const auto &copyAssignTask = static_cast <const CopyAssignHandle &> (unwrappedTask);
                        INFO ("Assigning copy of handle \"", copyAssignTask.sourceName, "\" to \"",
                             copyAssignTask.targetName, "\".");

                        auto sourceIterator = handles.find (copyAssignTask.sourceName);
                        REQUIRE (sourceIterator != handles.end ());

                        auto targetIterator = handles.find (copyAssignTask.targetName);
                        REQUIRE (targetIterator != handles.end ());

                        bool sourceValid = sourceIterator->second;
                        targetIterator->second = sourceIterator->second;
                        CHECK (sourceIterator->second == targetIterator->second);

                        if (sourceIterator == targetIterator)
                        {
                            HandleableSingletonResource *expected =
                                sourceValid ? HandleableSingletonResource::AcquireInstance () : nullptr;
                            CHECK (sourceIterator->second.Get () == expected);
                        }
                    }
                    else if constexpr (std::is_same_v <CurrentTask, MoveAssignHandle>)
                    {
                        const auto &moveAssignTask = static_cast <const MoveAssignHandle &> (unwrappedTask);
                        INFO ("Moving handle \"", moveAssignTask.sourceName, "\" into \"",
                             moveAssignTask.targetName, "\" using assignment.");

                        auto sourceIterator = handles.find (moveAssignTask.sourceName);
                        REQUIRE (sourceIterator != handles.end ());

                        auto targetIterator = handles.find (moveAssignTask.targetName);
                        REQUIRE (targetIterator != handles.end ());

                        bool sourceValid = sourceIterator->second;
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
                            CHECK (targetIterator->second == sourceValid);
                        }
                    }
                },
                task);

            bool anyValidHandles = false;
            for (const auto &[name, handle] : handles)
            {
                if (handle)
                {
                    anyValidHandles = true;
                    break;
                }
            }

            CHECK (HandleableSingletonResource::HasInstance () == anyValidHandles);
            if (anyValidHandles)
            {
                for (const auto &[name, handle] : handles)
                {
                    if (handle)
                    {
                        CHECK (handle.Get () == HandleableSingletonResource::AcquireInstance ());
                    }
                }
            }
        }

        handles.clear ();
        CHECK (HandleableSingletonResource::HasInstance () == false);
        HandleableSingletonResource::Cleanup ();
    }

    friend std::ostream &operator << (std::ostream &_output, const Seed &_seed)
    {
        _output << "Scenario: " << std::endl;
        for (const Task &task : _seed.tasks)
        {
            std::visit (
                [&_output] (const auto &unwrappedTask)
                {
                    using CurrentTask = std::decay_t <decltype (unwrappedTask)>;
                    if constexpr (std::is_same_v <CurrentTask, ConstructHandle>)
                    {
                        const auto &constructTask = static_cast <const ConstructHandle &> (unwrappedTask);
                        _output << "- Construct \"" << constructTask.name << "\"." << std::endl;
                    }
                    else if constexpr (std::is_same_v <CurrentTask, DestructHandle>)
                    {
                        const auto &destructTask = static_cast <const DestructHandle &> (unwrappedTask);
                        _output << "- Destruct \"" << destructTask.name << "\"." << std::endl;
                    }
                    else if constexpr (std::is_same_v <CurrentTask, CopyHandle>)
                    {
                        const auto &copyTask = static_cast <const CopyHandle &> (unwrappedTask);
                        _output << "- Copy \"" << copyTask.sourceName << "\" to \"" <<
                                copyTask.targetName << "\"." << std::endl;
                    }
                    else if constexpr (std::is_same_v <CurrentTask, MoveHandle>)
                    {
                        const auto &moveTask = static_cast <const MoveHandle &> (unwrappedTask);
                        _output << "- Move \"" << moveTask.sourceName << "\" into \"" <<
                                moveTask.targetName << "\"." << std::endl;
                    }
                    else if constexpr (std::is_same_v <CurrentTask, CopyAssignHandle>)
                    {
                        const auto &copyAssignTask = static_cast <const CopyAssignHandle &> (unwrappedTask);
                        _output << "- Assign copy of \"" << copyAssignTask.sourceName << "\" to \"" <<
                                copyAssignTask.targetName << "\"." << std::endl;
                    }
                    else if constexpr (std::is_same_v <CurrentTask, MoveAssignHandle>)
                    {
                        const auto &moveAssignTask = static_cast <const MoveAssignHandle &> (unwrappedTask);
                        _output << "- Move \"" << moveAssignTask.sourceName << "\" into \"" <<
                                moveAssignTask.targetName << "\" using assign operator." << std::endl;
                    }
                },
                task);
        }

        return _output;
    }

private:
    std::vector <Task> tasks;
};
} // namespace Emergence::Handling::Test

BEGIN_SUITE (HandleManagement)

TEST_CASE ()
{
    Emergence::Handling::Test::Seed (
        {
            Emergence::Handling::Test::ConstructHandle {"first"},
        }).GrowAndTest ();
}

TEST_CASE ()
{
    Emergence::Handling::Test::Seed (
        {
            Emergence::Handling::Test::ConstructHandle {"first"},
            Emergence::Handling::Test::DestructHandle {"first"},
        }).GrowAndTest ();
}

TEST_CASE ()
{
    Emergence::Handling::Test::Seed (
        {
            Emergence::Handling::Test::ConstructHandle {"first"},
            Emergence::Handling::Test::ConstructHandle {"second"},
            Emergence::Handling::Test::DestructHandle {"first"},
            Emergence::Handling::Test::DestructHandle {"second"},
        }).GrowAndTest ();
}

TEST_CASE ()
{
    Emergence::Handling::Test::Seed (
        {
            Emergence::Handling::Test::ConstructHandle {"first"},
            Emergence::Handling::Test::ConstructHandle {"second"},
            Emergence::Handling::Test::DestructHandle {"second"},
        }).GrowAndTest ();
}

TEST_CASE ()
{
    Emergence::Handling::Test::Seed (
        {
            Emergence::Handling::Test::ConstructHandle {"first"},
            Emergence::Handling::Test::CopyHandle {"first", "second"},
            Emergence::Handling::Test::DestructHandle {"first"},
        }).GrowAndTest ();
}

TEST_CASE ()
{
    Emergence::Handling::Test::Seed (
        {
            Emergence::Handling::Test::ConstructHandle {"first"},
            Emergence::Handling::Test::CopyHandle {"first", "second"},
            Emergence::Handling::Test::DestructHandle {"second"},
        }).GrowAndTest ();
}

TEST_CASE ()
{
    Emergence::Handling::Test::Seed (
        {
            Emergence::Handling::Test::ConstructHandle {"first"},
            Emergence::Handling::Test::MoveHandle {"first", "second"},
            Emergence::Handling::Test::DestructHandle {"first"},
        }).GrowAndTest ();
}

TEST_CASE ()
{
    Emergence::Handling::Test::Seed (
        {
            Emergence::Handling::Test::ConstructHandle {"first"},
            Emergence::Handling::Test::MoveHandle {"first", "second"},
            Emergence::Handling::Test::DestructHandle {"second"},
        }).GrowAndTest ();
}

TEST_CASE ()
{
    Emergence::Handling::Test::Seed (
        {
            Emergence::Handling::Test::ConstructHandle {"first"},
            Emergence::Handling::Test::MoveHandle {"first", "second"},
            Emergence::Handling::Test::ConstructHandle {"third"},
            Emergence::Handling::Test::DestructHandle {"second"},
        }).GrowAndTest ();
}

TEST_CASE ()
{
    Emergence::Handling::Test::Seed (
        {
            Emergence::Handling::Test::ConstructHandle {"first"},
            Emergence::Handling::Test::MoveHandle {"first", "second"},
            Emergence::Handling::Test::MoveHandle {"first", "third"},
            Emergence::Handling::Test::DestructHandle {"second"},
        }).GrowAndTest ();
}

TEST_CASE ()
{
    Emergence::Handling::Test::Seed (
        {
            Emergence::Handling::Test::ConstructHandle {"first"},
            Emergence::Handling::Test::MoveHandle {"first", "second"},
            Emergence::Handling::Test::CopyHandle {"first", "third"},
            Emergence::Handling::Test::DestructHandle {"second"},
        }).GrowAndTest ();
}

TEST_CASE ()
{
    Emergence::Handling::Test::Seed (
        {
            Emergence::Handling::Test::ConstructHandle {"first"},
            Emergence::Handling::Test::CopyAssignHandle {"first", "first"},
        }).GrowAndTest ();
}

TEST_CASE ()
{
    Emergence::Handling::Test::Seed (
        {
            Emergence::Handling::Test::ConstructHandle {"first"},
            Emergence::Handling::Test::MoveAssignHandle {"first", "first"},
        }).GrowAndTest ();
}

TEST_CASE ()
{
    Emergence::Handling::Test::Seed (
        {
            Emergence::Handling::Test::ConstructHandle {"first"},
            Emergence::Handling::Test::ConstructHandle {"second"},
            Emergence::Handling::Test::CopyAssignHandle {"first", "second"},
            Emergence::Handling::Test::DestructHandle {"first"},
        }).GrowAndTest ();
}

TEST_CASE ()
{
    Emergence::Handling::Test::Seed (
        {
            Emergence::Handling::Test::ConstructHandle {"first"},
            Emergence::Handling::Test::ConstructHandle {"second"},
            Emergence::Handling::Test::MoveAssignHandle {"first", "second"},
            Emergence::Handling::Test::DestructHandle {"first"},
        }).GrowAndTest ();
}

TEST_CASE ()
{
    Emergence::Handling::Test::Seed (
        {
            Emergence::Handling::Test::ConstructHandle {"first"},
            Emergence::Handling::Test::ConstructHandle {"second"},
            Emergence::Handling::Test::MoveAssignHandle {"first", "second"},
            Emergence::Handling::Test::DestructHandle {"second"},
        }).GrowAndTest ();
}

TEST_CASE ()
{
    Emergence::Handling::Test::Seed (
        {
            Emergence::Handling::Test::ConstructHandle {"first"},
            Emergence::Handling::Test::ConstructHandle {"second"},
            Emergence::Handling::Test::ConstructHandle {"third"},
            Emergence::Handling::Test::MoveAssignHandle {"first", "second"},
            Emergence::Handling::Test::MoveAssignHandle {"first", "third"},
            Emergence::Handling::Test::DestructHandle {"second"},
        }).GrowAndTest ();
}

TEST_CASE ()
{
    Emergence::Handling::Test::Seed (
        {
            Emergence::Handling::Test::ConstructHandle {"first"},
            Emergence::Handling::Test::ConstructHandle {"second"},
            Emergence::Handling::Test::ConstructHandle {"third"},
            Emergence::Handling::Test::MoveAssignHandle {"first", "second"},
            Emergence::Handling::Test::CopyAssignHandle {"first", "third"},
            Emergence::Handling::Test::DestructHandle {"second"},
        }).GrowAndTest ();
}

END_SUITE