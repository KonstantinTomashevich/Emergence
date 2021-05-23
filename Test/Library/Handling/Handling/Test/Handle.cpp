#include <unordered_map>
#include <variant>

#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/format.hpp>

#include <Handling/Handle.hpp>
#include <Handling/HandleableBase.hpp>

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
                        BOOST_TEST_MESSAGE (
                            boost::format ("Constructing handle \"%1%\" from raw instance.") % constructTask.name);

                        BOOST_REQUIRE (handles.find (constructTask.name) == handles.end ());
                        auto[iterator, emplaceResult] = handles.emplace (
                            constructTask.name, HandleableSingletonResource::AcquireInstance ());

                        BOOST_REQUIRE (emplaceResult);
                        BOOST_CHECK (iterator->second);
                    }
                    else if constexpr (std::is_same_v <CurrentTask, DestructHandle>)
                    {
                        const auto &destructTask = static_cast <const DestructHandle &> (unwrappedTask);
                        BOOST_TEST_MESSAGE (
                            boost::format ("Destructing handle \"%1%\".") % destructTask.name);

                        auto iterator = handles.find (destructTask.name);
                        BOOST_REQUIRE (iterator != handles.end ());
                        handles.erase (iterator);
                    }
                    else if constexpr (std::is_same_v <CurrentTask, CopyHandle>)
                    {
                        const auto &copyTask = static_cast <const CopyHandle &> (unwrappedTask);
                        BOOST_TEST_MESSAGE (boost::format ("Copying handle \"%1%\" to \"%2%\".") %
                                            copyTask.sourceName % copyTask.targetName);

                        auto sourceIterator = handles.find (copyTask.sourceName);
                        BOOST_REQUIRE (sourceIterator != handles.end ());
                        BOOST_REQUIRE (handles.find (copyTask.targetName) == handles.end ());

                        auto[iterator, emplaceResult] = handles.emplace (copyTask.targetName, sourceIterator->second);
                        BOOST_REQUIRE (emplaceResult);
                        BOOST_CHECK (iterator->second == sourceIterator->second);
                    }
                    else if constexpr (std::is_same_v <CurrentTask, MoveHandle>)
                    {
                        const auto &moveTask = static_cast <const MoveHandle &> (unwrappedTask);
                        BOOST_TEST_MESSAGE (boost::format ("Moving handle \"%1%\" into \"%2%\".") %
                                            moveTask.sourceName % moveTask.targetName);

                        auto sourceIterator = handles.find (moveTask.sourceName);
                        BOOST_REQUIRE (sourceIterator != handles.end ());
                        BOOST_REQUIRE (handles.find (moveTask.targetName) == handles.end ());

                        bool sourceValid = sourceIterator->second;
                        auto[iterator, emplaceResult] = handles.emplace (
                            moveTask.targetName, std::move (sourceIterator->second));

                        BOOST_REQUIRE (emplaceResult);
                        BOOST_CHECK (!sourceIterator->second);
                        BOOST_CHECK (iterator->second || !sourceValid);
                    }
                    else if constexpr (std::is_same_v <CurrentTask, CopyAssignHandle>)
                    {
                        const auto &copyAssignTask = static_cast <const CopyAssignHandle &> (unwrappedTask);
                        BOOST_TEST_MESSAGE (boost::format ("Assigning copy of handle \"%1%\" to \"%2%\".") %
                                            copyAssignTask.sourceName % copyAssignTask.targetName);

                        auto sourceIterator = handles.find (copyAssignTask.sourceName);
                        BOOST_REQUIRE (sourceIterator != handles.end ());

                        auto targetIterator = handles.find (copyAssignTask.targetName);
                        BOOST_REQUIRE (targetIterator != handles.end ());

                        bool sourceValid = sourceIterator->second;
                        targetIterator->second = sourceIterator->second;
                        BOOST_CHECK (sourceIterator->second == targetIterator->second);

                        if (sourceIterator == targetIterator)
                        {
                            HandleableSingletonResource *expected =
                                sourceValid ? HandleableSingletonResource::AcquireInstance () : nullptr;
                            BOOST_CHECK (sourceIterator->second.Get () == expected);
                        }
                    }
                    else if constexpr (std::is_same_v <CurrentTask, MoveAssignHandle>)
                    {
                        const auto &moveAssignTask = static_cast <const MoveAssignHandle &> (unwrappedTask);
                        BOOST_TEST_MESSAGE (boost::format ("Moving handle \"%1%\" into \"%2%\" using assignment.") %
                                            moveAssignTask.sourceName % moveAssignTask.targetName);

                        auto sourceIterator = handles.find (moveAssignTask.sourceName);
                        BOOST_REQUIRE (sourceIterator != handles.end ());

                        auto targetIterator = handles.find (moveAssignTask.targetName);
                        BOOST_REQUIRE (targetIterator != handles.end ());

                        bool sourceValid = sourceIterator->second;
                        targetIterator->second = std::move (sourceIterator->second);

                        if (sourceIterator == targetIterator)
                        {
                            HandleableSingletonResource *expected =
                                sourceValid ? HandleableSingletonResource::AcquireInstance () : nullptr;
                            BOOST_CHECK (sourceIterator->second.Get () == expected);
                        }
                        else
                        {
                            BOOST_CHECK (!sourceIterator->second);
                            BOOST_CHECK (targetIterator->second || !sourceValid);
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

            BOOST_CHECK_EQUAL (HandleableSingletonResource::HasInstance (), anyValidHandles);
            if (anyValidHandles)
            {
                for (const auto &[name, handle] : handles)
                {
                    if (handle)
                    {
                        BOOST_CHECK_EQUAL (handle.Get (), HandleableSingletonResource::AcquireInstance ());
                    }
                }
            }
        }

        handles.clear ();
        BOOST_CHECK_EQUAL (HandleableSingletonResource::HasInstance (), false);
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

BOOST_AUTO_TEST_SUITE (Handle)

BOOST_DATA_TEST_CASE (
    TestRoutine, boost::unit_test::data::monomorphic::collection (
    std::vector <Emergence::Handling::Test::Seed> {
        Emergence::Handling::Test::Seed (
            {
                Emergence::Handling::Test::ConstructHandle {"first"},
            }),

        Emergence::Handling::Test::Seed (
            {
                Emergence::Handling::Test::ConstructHandle {"first"},
                Emergence::Handling::Test::DestructHandle {"first"},
            }),

        Emergence::Handling::Test::Seed (
            {
                Emergence::Handling::Test::ConstructHandle {"first"},
                Emergence::Handling::Test::ConstructHandle {"second"},
                Emergence::Handling::Test::DestructHandle {"first"},
                Emergence::Handling::Test::DestructHandle {"second"},
            }),

        Emergence::Handling::Test::Seed (
            {
                Emergence::Handling::Test::ConstructHandle {"first"},
                Emergence::Handling::Test::ConstructHandle {"second"},
                Emergence::Handling::Test::DestructHandle {"second"},
            }),

        Emergence::Handling::Test::Seed (
            {
                Emergence::Handling::Test::ConstructHandle {"first"},
                Emergence::Handling::Test::CopyHandle {"first", "second"},
                Emergence::Handling::Test::DestructHandle {"first"},
            }),

        Emergence::Handling::Test::Seed (
            {
                Emergence::Handling::Test::ConstructHandle {"first"},
                Emergence::Handling::Test::CopyHandle {"first", "second"},
                Emergence::Handling::Test::DestructHandle {"second"},
            }),

        Emergence::Handling::Test::Seed (
            {
                Emergence::Handling::Test::ConstructHandle {"first"},
                Emergence::Handling::Test::MoveHandle {"first", "second"},
                Emergence::Handling::Test::DestructHandle {"first"},
            }),

        Emergence::Handling::Test::Seed (
            {
                Emergence::Handling::Test::ConstructHandle {"first"},
                Emergence::Handling::Test::MoveHandle {"first", "second"},
                Emergence::Handling::Test::DestructHandle {"second"},
            }),

        Emergence::Handling::Test::Seed (
            {
                Emergence::Handling::Test::ConstructHandle {"first"},
                Emergence::Handling::Test::MoveHandle {"first", "second"},
                Emergence::Handling::Test::ConstructHandle {"third"},
                Emergence::Handling::Test::DestructHandle {"second"},
            }),

        Emergence::Handling::Test::Seed (
            {
                Emergence::Handling::Test::ConstructHandle {"first"},
                Emergence::Handling::Test::MoveHandle {"first", "second"},
                Emergence::Handling::Test::MoveHandle {"first", "third"},
                Emergence::Handling::Test::DestructHandle {"second"},
            }),

        Emergence::Handling::Test::Seed (
            {
                Emergence::Handling::Test::ConstructHandle {"first"},
                Emergence::Handling::Test::MoveHandle {"first", "second"},
                Emergence::Handling::Test::CopyHandle {"first", "third"},
                Emergence::Handling::Test::DestructHandle {"second"},
            }),

        Emergence::Handling::Test::Seed (
            {
                Emergence::Handling::Test::ConstructHandle {"first"},
                Emergence::Handling::Test::CopyAssignHandle {"first", "first"},
            }),

        Emergence::Handling::Test::Seed (
            {
                Emergence::Handling::Test::ConstructHandle {"first"},
                Emergence::Handling::Test::MoveAssignHandle {"first", "first"},
            }),

        Emergence::Handling::Test::Seed (
            {
                Emergence::Handling::Test::ConstructHandle {"first"},
                Emergence::Handling::Test::ConstructHandle {"second"},
                Emergence::Handling::Test::CopyAssignHandle {"first", "second"},
                Emergence::Handling::Test::DestructHandle {"first"},
            }),

        Emergence::Handling::Test::Seed (
            {
                Emergence::Handling::Test::ConstructHandle {"first"},
                Emergence::Handling::Test::ConstructHandle {"second"},
                Emergence::Handling::Test::MoveAssignHandle {"first", "second"},
                Emergence::Handling::Test::DestructHandle {"first"},
            }),

        Emergence::Handling::Test::Seed (
            {
                Emergence::Handling::Test::ConstructHandle {"first"},
                Emergence::Handling::Test::ConstructHandle {"second"},
                Emergence::Handling::Test::MoveAssignHandle {"first", "second"},
                Emergence::Handling::Test::DestructHandle {"second"},
            }),

        Emergence::Handling::Test::Seed (
            {
                Emergence::Handling::Test::ConstructHandle {"first"},
                Emergence::Handling::Test::ConstructHandle {"second"},
                Emergence::Handling::Test::ConstructHandle {"third"},
                Emergence::Handling::Test::MoveAssignHandle {"first", "second"},
                Emergence::Handling::Test::MoveAssignHandle {"first", "third"},
                Emergence::Handling::Test::DestructHandle {"second"},
            }),

        Emergence::Handling::Test::Seed (
            {
                Emergence::Handling::Test::ConstructHandle {"first"},
                Emergence::Handling::Test::ConstructHandle {"second"},
                Emergence::Handling::Test::ConstructHandle {"third"},
                Emergence::Handling::Test::MoveAssignHandle {"first", "second"},
                Emergence::Handling::Test::CopyAssignHandle {"first", "third"},
                Emergence::Handling::Test::DestructHandle {"second"},
            }),
    }))
{
    sample.GrowAndTest ();
}

BOOST_AUTO_TEST_SUITE_END ()