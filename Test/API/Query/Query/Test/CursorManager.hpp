#pragma once

#include <string>
#include <unordered_map>
#include <variant>

#include <Query/Test/Scenario.hpp>

#include <StandardLayout/Mapping.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Query::Test
{
/// \brief Implements cursor management and cursor task execution for cursors with standard API.
/// \see EMERGENCE_READ_CURSOR_OPERATIONS
/// \see EMERGENCE_EDIT_CURSOR_OPERATIONS
/// \details Singleton-value cursors, that do not have increment, are also supported.
template <typename... Cursors>
struct CursorManager
{
    struct CursorData final
    {
        std::variant <Cursors...> cursor;
        StandardLayout::Mapping objectMapping;
    };

    std::unordered_map <std::string, CursorData> cursors;
};

template <typename T>
concept ReturnsEditablePointer =
requires (T _cursor) {
    { *_cursor } -> std::convertible_to <void *>;
};

template <typename T>
concept AllowsObjectDeletion =
requires (T _cursor) {
    { ~_cursor };
};

template <typename T>
concept Movable =
requires (T _cursor) {
    { ++_cursor } -> std::convertible_to <T &>;
};

template <typename... Cursors>
void ExecuteTask (CursorManager <Cursors...> &_manager, const Tasks::CursorCheck &_task)
{
    auto iterator = _manager.cursors.find (_task.name);
    REQUIRE_WITH_MESSAGE (iterator != _manager.cursors.end (),
                          "There should be cursor with name \"", _task.name, "\".");

    std::visit (
        [&_task, _mapping {iterator->second.objectMapping}] (auto &_cursor)
        {
            const void *object = *_cursor;
            if (_task.expectedObject)
            {
                if (object)
                {
                    bool equal = memcmp (object, _task.expectedObject, _mapping.GetObjectSize ()) == 0u;
                    CHECK_WITH_MESSAGE (
                        equal,
                        "Expected and pointed objects should be equal!\nObject: ", ObjectToString (_mapping, object),
                        "\nExpected object: ", ObjectToString (_mapping, _task.expectedObject));
                }
                else
                {
                    CHECK_WITH_MESSAGE (false, "Cursor should not be empty!");
                }
            }
            else if (object)
            {
                CHECK_WITH_MESSAGE (false, "Cursor should be empty!");
            }
        },
        iterator->second.cursor);
}

template <typename... Cursors>
void ExecuteTask (CursorManager <Cursors...> &_manager, const Tasks::CursorCheckAllOrdered &_task)
{
    auto iterator = _manager.cursors.find (_task.name);
    REQUIRE_WITH_MESSAGE (iterator != _manager.cursors.end (),
                          "There should be cursor with name \"", _task.name, "\".");

    std::visit (
        [&_task, _mapping {iterator->second.objectMapping}] (auto &_cursor)
        {
            if constexpr (Movable <std::decay_t <decltype (_cursor)>>)
            {
                std::size_t position = 0u;
                const void *object;

                while ((object = *_cursor) || position < _task.expectedObjects.size ())
                {
                    const void *expected;
                    if (position < _task.expectedObjects.size ())
                    {
                        expected = _task.expectedObjects[position];
                    }
                    else
                    {
                        expected = nullptr;
                    }

                    if (object && expected)
                    {
                        bool equal = memcmp (object, expected, _mapping.GetObjectSize ()) == 0;
                        CHECK_WITH_MESSAGE (
                            equal, "Checking that received object ", object, " and expected object ", expected,
                            " at position ", position, " are equal.\nReceived: ", ObjectToString (_mapping, object),
                            "\nExpected: ", ObjectToString (_mapping, expected));
                    }
                    else if (object)
                    {
                        CHECK_WITH_MESSAGE (
                            false, "Expecting nothing at position ", position, ", receiving ",
                            ObjectToString (_mapping, object));
                    }
                    else
                    {
                        CHECK_WITH_MESSAGE (
                            false, "Expecting ", expected, " at position ", position, ", but receiving nothing");
                    }

                    if (object)
                    {
                        ++_cursor;
                    }

                    ++position;
                }
            }
            else
            {
                REQUIRE_WITH_MESSAGE(false, "Cursor with name \"", _task.name, "\" must be movable!");
            }
        },
        iterator->second.cursor);
}

template <typename... Cursors>
void ExecuteTask (CursorManager <Cursors...> &_manager, const Tasks::CursorCheckAllUnordered &_task)
{
    auto iterator = _manager.cursors.find (_task.name);
    REQUIRE_WITH_MESSAGE (iterator != _manager.cursors.end (), "There should be cursor with name \"", _task.name,
                          "\".");

    std::visit (
        [&_task, _mapping {iterator->second.objectMapping}] (auto &_cursor)
        {
            if constexpr (Movable <std::decay_t <decltype (_cursor)>>)
            {
                std::vector <const void *> objects;
                while (const void *object = *_cursor)
                {
                    objects.emplace_back (object);
                    ++_cursor;
                }

                // Brute force counting is the most efficient solution there,
                // because tests check small vectors of objects, usually not more than 5.
                auto Count = [_mapping] (const std::vector <const void *> &_objects, const void *_objectToSearch)
                {
                    std::size_t count = 0u;
                    for (const void *_otherObject : _objects)
                    {
                        if (memcmp (_objectToSearch, _otherObject, _mapping.GetObjectSize ()) == 0)
                        {
                            ++count;
                        }
                    }

                    return count;
                };

                for (const void *objectFromCursor : objects)
                {
                    const std::size_t countInCursor = Count (objects, objectFromCursor);
                    const std::size_t countExpected = Count (_task.expectedObjects, objectFromCursor);
                    CHECK_EQUAL (countInCursor, countExpected);

                    if (countInCursor != countExpected)
                    {
                        LOG ("Checked object: ", ObjectToString (_mapping, objectFromCursor));
                    }
                }
            }
            else
            {
                REQUIRE_WITH_MESSAGE(false, "Cursor with name \"", _task.name, "\" must be movable!");
            }
        },
        iterator->second.cursor);
}

template <typename... Cursors>
void ExecuteTask (CursorManager <Cursors...> &_manager, const Tasks::CursorEdit &_task)
{
    auto iterator = _manager.cursors.find (_task.name);
    REQUIRE_WITH_MESSAGE (
        iterator != _manager.cursors.end (),
        "There should be cursor with name \"", _task.name, "\".");

    std::visit (
        [&_task, _mapping {iterator->second.objectMapping}] (auto &_cursor)
        {
            if constexpr (ReturnsEditablePointer <std::decay_t <decltype (_cursor)>>)
            {
                void *object = *_cursor;
                CHECK_WITH_MESSAGE (object, "Cursor should not be empty.");
                REQUIRE_WITH_MESSAGE (_task.copyFromObject, "New value source must not be null pointer!");

                if (object)
                {
                    memcpy (object, _task.copyFromObject, _mapping.GetObjectSize ());
                }
            }
            else
            {
                REQUIRE_WITH_MESSAGE (false, "Cursor ", _task.name, " should be editable.");
            }
        },
        iterator->second.cursor);
}

template <typename... Cursors>
void ExecuteTask (CursorManager <Cursors...> &_manager, const Tasks::CursorIncrement &_task)
{
    auto iterator = _manager.cursors.find (_task.name);
    REQUIRE_WITH_MESSAGE (
        iterator != _manager.cursors.end (),
        "There should be cursor with name \"", _task.name, "\".");

    std::visit (
        [&_task] (auto &_cursor)
        {
            if constexpr (Movable <std::decay_t <decltype (_cursor)>>)
            {
                ++_cursor;
            }
            else
            {
                REQUIRE_WITH_MESSAGE(false, "Cursor with name \"", _task.name, "\" must be movable!");
            }
        },
        iterator->second.cursor);
}

template <typename... Cursors>
void ExecuteTask (CursorManager <Cursors...> &_manager, const Tasks::CursorDeleteObject &_task)
{
    auto iterator = _manager.cursors.find (_task.name);
    REQUIRE_WITH_MESSAGE (iterator != _manager.cursors.end (), "There should be cursor with name \"", _task.name,
                          "\".");

    std::visit (
        [&_task] (auto &_cursor)
        {
            if constexpr (AllowsObjectDeletion <std::decay_t <decltype (_cursor)>>)
            {
                ~_cursor;
            }
            else
            {
                REQUIRE_WITH_MESSAGE (false, "Cursor ", _task.name, " should allow object deletion.");
            }
        },
        iterator->second.cursor);
}

template <typename... Cursors>
void ExecuteTask (CursorManager <Cursors...> &_manager, const Tasks::CursorCopy &_task)
{
    auto iterator = _manager.cursors.find (_task.sourceName);
    REQUIRE_WITH_MESSAGE (
        iterator != _manager.cursors.end (),
        "There should be cursor with name \"", _task.sourceName, "\".");

    REQUIRE_WITH_MESSAGE (
        _manager.cursors.find (_task.targetName) == _manager.cursors.end (),
        "There should be no cursor with name \"", _task.targetName, "\".");

    std::visit (
        [&_manager, &_task, _mapping {iterator->second.objectMapping}] (auto &_cursor)
        {
            if constexpr (std::copy_constructible <std::decay_t <decltype (_cursor)>>)
            {
                _manager.cursors.emplace (
                    _task.targetName, typename CursorManager <Cursors...>::CursorData {_cursor, _mapping});
            }
            else
            {
                REQUIRE_WITH_MESSAGE (false, "Cursor ", _task.sourceName, " should be copy constructable.");
            }
        },
        iterator->second.cursor);
}

template <typename... Cursors>
void ExecuteTask (CursorManager <Cursors...> &_manager, const Tasks::CursorMove &_task)
{
    auto iterator = _manager.cursors.find (_task.sourceName);
    REQUIRE_WITH_MESSAGE (
        iterator != _manager.cursors.end (),
        "There should be cursor with name \"", _task.sourceName, "\".");

    REQUIRE_WITH_MESSAGE (
        _manager.cursors.find (_task.targetName) == _manager.cursors.end (),
        "There should be no cursor with name \"", _task.targetName, "\".");

    std::visit (
        [&_manager, &_task, _mapping {iterator->second.objectMapping}] (auto &_cursor)
        {
            _manager.cursors.emplace (
                _task.targetName, typename CursorManager <Cursors...>::CursorData {std::move (_cursor), _mapping});
        },
        iterator->second.cursor);
}

template <typename... Cursors>
void ExecuteTask (CursorManager <Cursors...> &_manager, const Tasks::CursorClose &_task)
{
    auto iterator = _manager.cursors.find (_task.name);
    REQUIRE_WITH_MESSAGE (
        iterator != _manager.cursors.end (),
        "There should be cursor with name \"", _task.name, "\".");

    _manager.cursors.erase (iterator);
}

template <typename CursorType, typename... Cursors>
void AddCursor (CursorManager <Cursors...> &_manager, std::string _name,
                const StandardLayout::Mapping &_objectMapping, CursorType &&_cursor)
{
    REQUIRE_WITH_MESSAGE (
        _manager.cursors.find (_name) == _manager.cursors.end (), "There should be no cursor with name \"", _name,
        "\"");

    _manager.cursors.emplace (
        std::move (_name), typename CursorManager <Cursors...>::CursorData {std::move (_cursor), _objectMapping});
}

std::string ObjectToString (const StandardLayout::Mapping &_mapping, const void *_object)
{
    const auto *current = static_cast <const uint8_t *> (_object);
    const auto *end = current + _mapping.GetObjectSize ();
    std::string result;

    while (current != end)
    {
        result += std::to_string (static_cast <std::size_t> (*current)) + " ";
        ++current;
    }

    return result;
}
} // namespace Emergence::Query::Test