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
class CursorManager
{
public:
    CursorManager () = default;

    void ExecuteTask (const Tasks::CursorCheck &_task);

    void ExecuteTask (const Tasks::CursorCheckAllOrdered &_task);

    void ExecuteTask (const Tasks::CursorCheckAllUnordered &_task);

    void ExecuteTask (const Tasks::CursorEdit &_task);

    void ExecuteTask (const Tasks::CursorIncrement &_task);

    void ExecuteTask (const Tasks::CursorDeleteObject &_task);

    void ExecuteTask (const Tasks::CursorCopy &_task);

    void ExecuteTask (const Tasks::CursorMove &_task);

    void ExecuteTask (const Tasks::CursorClose &_task);

    template <typename CursorType>
    void Add (std::string _name, const StandardLayout::Mapping &_objectMapping, CursorType &&_cursor);

private:
    struct Cursor final
    {
        std::variant <Cursors...> cursor;
        StandardLayout::Mapping objectMapping;
    };

    std::string ObjectToString (const StandardLayout::Mapping &_mapping, const void *_object) const;

    std::unordered_map <std::string, Cursor> cursors;
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
void CursorManager <Cursors...>::ExecuteTask (const Tasks::CursorCheck &_task)
{
    auto iterator = cursors.find (_task.name);
    REQUIRE_WITH_MESSAGE (iterator != cursors.end (), "There should be cursor with name \"", _task.name, "\".");

    std::visit (
        [this, &_task, _mapping {iterator->second.objectMapping}] (auto &_cursor)
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
void CursorManager <Cursors...>::ExecuteTask (const Tasks::CursorCheckAllOrdered &_task)
{
    auto iterator = cursors.find (_task.name);
    REQUIRE_WITH_MESSAGE (iterator != cursors.end (), "There should be cursor with name \"", _task.name, "\".");

    std::visit (
        [this, &_task, _mapping {iterator->second.objectMapping}] (auto &_cursor)
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
void CursorManager <Cursors...>::ExecuteTask (const Tasks::CursorCheckAllUnordered &_task)
{
    auto iterator = cursors.find (_task.name);
    REQUIRE_WITH_MESSAGE (iterator != cursors.end (), "There should be cursor with name \"", _task.name, "\".");

    std::visit (
        [this, &_task, _mapping {iterator->second.objectMapping}] (auto &_cursor)
        {
            if constexpr (Movable <std::decay_t <decltype (_cursor)>>)
            {
                std::vector <const void *> objects;
                while (const void *object = *_cursor)
                {
                    objects.emplace_back (object);
                    ++_cursor;
                }

                CHECK_EQUAL (objects.size (), _task.expectedObjects.size ());
                auto Search = [_mapping] (const std::vector <const void *> &_objects, const void *_objectToSearch)
                {
                    return std::find_if (
                        _objects.begin (), _objects.end (),
                        [_objectToSearch, &_mapping] (const void *_otherObject)
                        {
                            return memcmp (_objectToSearch, _otherObject, _mapping.GetObjectSize ()) == 0u;
                        });
                };

                for (const void *objectFromCursor : objects)
                {
                    auto iterator = Search (_task.expectedObjects, objectFromCursor);
                    if (iterator == _task.expectedObjects.end ())
                    {
                        CHECK_WITH_MESSAGE (
                            false, "Searching for object from cursor in expected objects list. Object: ",
                            ObjectToString (_mapping, objectFromCursor));
                    }
                }

                for (const void *expectedObject : objects)
                {
                    auto iterator = Search (objects, expectedObject);
                    if (iterator == objects.end ())
                    {
                        CHECK_WITH_MESSAGE (
                            false, "Searching for expected object in received objects list. Object: ",
                            ObjectToString (_mapping, expectedObject));
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
void CursorManager <Cursors...>::ExecuteTask (const Tasks::CursorEdit &_task)
{
    auto iterator = cursors.find (_task.name);
    REQUIRE_WITH_MESSAGE (
        iterator != cursors.end (),
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
void CursorManager <Cursors...>::ExecuteTask (const Tasks::CursorIncrement &_task)
{
    auto iterator = cursors.find (_task.name);
    REQUIRE_WITH_MESSAGE (
        iterator != cursors.end (),
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
void CursorManager <Cursors...>::ExecuteTask (const Tasks::CursorDeleteObject &_task)
{
    auto iterator = cursors.find (_task.name);
    REQUIRE_WITH_MESSAGE (iterator != cursors.end (), "There should be cursor with name \"", _task.name, "\".");

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
void CursorManager <Cursors...>::ExecuteTask (const Tasks::CursorCopy &_task)
{
    auto iterator = cursors.find (_task.sourceName);
    REQUIRE_WITH_MESSAGE (
        iterator != cursors.end (),
        "There should be cursor with name \"", _task.sourceName, "\".");

    REQUIRE_WITH_MESSAGE (
        cursors.find (_task.targetName) == cursors.end (),
        "There should be no cursor with name \"", _task.targetName, "\".");

    std::visit (
        [this, &_task, _mapping {iterator->second.objectMapping}] (auto &_cursor)
        {
            if constexpr (std::copy_constructible <std::decay_t <decltype (_cursor)>>)
            {
                cursors.emplace (_task.targetName, Cursor {_cursor, _mapping});
            }
            else
            {
                REQUIRE_WITH_MESSAGE (false, "Cursor ", _task.sourceName, " should be copy constructable.");
            }
        },
        iterator->second.cursor);
}

template <typename... Cursors>
void CursorManager <Cursors...>::ExecuteTask (const Tasks::CursorMove &_task)
{
    auto iterator = cursors.find (_task.sourceName);
    REQUIRE_WITH_MESSAGE (
        iterator != cursors.end (),
        "There should be cursor with name \"", _task.sourceName, "\".");

    REQUIRE_WITH_MESSAGE (
        cursors.find (_task.targetName) == cursors.end (),
        "There should be no cursor with name \"", _task.targetName, "\".");

    std::visit (
        [this, &_task, _mapping {iterator->second.objectMapping}] (auto &_cursor)
        {
            cursors.emplace (_task.targetName, Cursor {std::move (_cursor), _mapping});
        },
        iterator->second.cursor);
}

template <typename... Cursors>
void CursorManager <Cursors...>::ExecuteTask (const Tasks::CursorClose &_task)
{
    auto iterator = cursors.find (_task.name);
    REQUIRE_WITH_MESSAGE (
        iterator != cursors.end (),
        "There should be cursor with name \"", _task.name, "\".");

    cursors.erase (iterator);
}

template <typename... Cursors>
template <typename CursorType>
void
CursorManager <Cursors...>::Add (std::string _name, const StandardLayout::Mapping &_objectMapping, CursorType &&_cursor)
{
    REQUIRE_WITH_MESSAGE (
        cursors.find (_name) == cursors.end (), "There should be no cursor with name \"", _name, "\"");
    cursors.emplace (std::move (_name), Cursor {std::move (_cursor), _objectMapping});
}

template <typename... Cursors>
std::string CursorManager <Cursors...>::ObjectToString (
    const StandardLayout::Mapping &_mapping, const void *_object) const
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