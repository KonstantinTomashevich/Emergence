#pragma once

#include <string>
#include <unordered_map>

#include <Context/Extension/ObjectStorage.hpp>

#include <Query/Test/Scenario.hpp>

#include <StandardLayout/Mapping.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Query::Test
{
template <typename Cursor>
requires std::is_copy_constructible_v <Cursor>
Cursor CopyCursor (const Cursor &_other)
{
    return _other;
}

template <typename... Variants>
std::variant <Variants...> CopyCursor (const std::variant <Variants...> &_other)
{
    return std::visit (
        [] (const auto &_cursor) -> std::variant <Variants...>
        {
            if constexpr (std::is_copy_constructible_v <std::decay_t <decltype (_cursor)>>)
            {
                return _cursor;
            }
            else
            {
                REQUIRE_WITH_MESSAGE (false, "Required cursor type is not copy constructable!");
                // Should never be thrown, because check above interrupts execution.
                // Added to suppress missing return warnings.
                throw std::runtime_error ("Required cursor type is not copy constructable!");
            }
        },
        _other);
}

template <typename Cursor>
struct CursorData final
{
    CursorData (Cursor _cursor, StandardLayout::Mapping _objectMapping)
        : cursor (std::move (_cursor)),
          objectMapping (std::move (_objectMapping))
    {
    }

    CursorData (const CursorData &_other)
        : cursor (CopyCursor (_other.cursor)),
          objectMapping (_other.objectMapping)
    {
    }

    CursorData (CursorData &&_other) noexcept = default;

    Cursor cursor;
    StandardLayout::Mapping objectMapping;
};

/// \brief Stores cursor objects and implements cursor task execution for cursors with standard API.
/// \see EMERGENCE_READ_CURSOR_OPERATIONS
/// \see EMERGENCE_EDIT_CURSOR_OPERATIONS
/// \details Singleton-value cursors, that do not have increment, are also supported.
template <typename Cursor>
struct CursorStorage : public Context::Extension::ObjectStorage <CursorData <Cursor>>
{
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

template <typename Cursor>
void ExecuteTask (CursorStorage <Cursor> &_storage, const Tasks::CursorCheck &_task)
{
    CursorData <Cursor> &cursorData = GetObject (_storage, _task.name);
    std::visit (
        [&_task, _mapping {cursorData.objectMapping}] (auto &_cursor)
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
        cursorData.cursor);
}

template <typename Cursor>
void ExecuteTask (CursorStorage <Cursor> &_storage, const Tasks::CursorCheckAllOrdered &_task)
{
    CursorData <Cursor> &cursorData = GetObject (_storage, _task.name);
    std::visit (
        [&_task, _mapping {cursorData.objectMapping}] (auto &_cursor)
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
        cursorData.cursor);
}

template <typename Cursor>
void ExecuteTask (CursorStorage <Cursor> &_storage, const Tasks::CursorCheckAllUnordered &_task)
{
    CursorData <Cursor> &cursorData = GetObject (_storage, _task.name);
    std::visit (
        [&_task, _mapping {cursorData.objectMapping}] (auto &_cursor)
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
        cursorData.cursor);
}

template <typename Cursor>
void ExecuteTask (CursorStorage <Cursor> &_storage, const Tasks::CursorEdit &_task)
{
    CursorData <Cursor> &cursorData = GetObject (_storage, _task.name);
    std::visit (
        [&_task, _mapping {cursorData.objectMapping}] (auto &_cursor)
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
        cursorData.cursor);
}

template <typename Cursor>
void ExecuteTask (CursorStorage <Cursor> &_storage, const Tasks::CursorIncrement &_task)
{
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
        GetObject (_storage, _task.name).cursor);
}

template <typename Cursor>
void ExecuteTask (CursorStorage <Cursor> &_storage, const Tasks::CursorDeleteObject &_task)
{
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
        GetObject (_storage, _task.name).cursor);
}

template <typename Cursor>
void ExecuteTask (CursorStorage <Cursor> &_storage, const Tasks::CursorClose &_task)
{
    RemoveObject (_storage, _task.name);
}

template <typename Cursor>
void AddObject (CursorStorage <Cursor> &_storage, std::string _name,
                const StandardLayout::Mapping &_objectMapping, Cursor &&_cursor)
{
    AddObject (_storage, _name, CursorData <Cursor> {std::move (_cursor), _objectMapping});
}

template <typename Cursor>
void GetCursor (CursorStorage <Cursor> &_storage, std::string _name)
{
    return GetObject (_storage, _name).cursor;
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