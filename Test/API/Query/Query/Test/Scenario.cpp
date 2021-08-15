#include <iostream>

#include <Query/Test/Scenario.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Query::Test
{
Sources::Volumetric::SupportedValue::SupportedValue (int8_t _value) noexcept
    : int8 (_value)
{
}

Sources::Volumetric::SupportedValue::SupportedValue (int16_t _value) noexcept
    : int16 (_value)
{
}

Sources::Volumetric::SupportedValue::SupportedValue (int32_t _value) noexcept
    : int32 (_value)
{
}

Sources::Volumetric::SupportedValue::SupportedValue (int64_t _value) noexcept
    : int64 (_value)
{
}

Sources::Volumetric::SupportedValue::SupportedValue (uint8_t _value) noexcept
    : uint8 (_value)
{
}

Sources::Volumetric::SupportedValue::SupportedValue (uint16_t _value) noexcept
    : uint16 (_value)
{
}

Sources::Volumetric::SupportedValue::SupportedValue (uint32_t _value) noexcept
    : uint32 (_value)
{
}

Sources::Volumetric::SupportedValue::SupportedValue (uint64_t _value) noexcept
    : uint64 (_value)
{
}

Sources::Volumetric::SupportedValue::SupportedValue (float _value) noexcept
    : floating (_value)
{
}

Sources::Volumetric::SupportedValue::SupportedValue (double _value) noexcept
    : doubleFloating (_value)
{
}

namespace Sources
{
std::ostream &operator << (std::ostream &_output, const Sources::Volumetric::SupportedValue &_value)
{
    // To increase log readability we print only types, that are used by volumetric queries tests.
    return _output << "{" << _value.floating << "f or " << _value.int16 << "i16}";
}
} // namespace Sources

namespace Tasks
{
std::ostream &operator << (std::ostream &_output, const QuerySingletonToRead &_task)
{
    return _output << "Query singleton from \"" << _task.sourceName << "\" and save read only cursor as \"" <<
                   _task.cursorName << "\".";
}

std::ostream &operator << (std::ostream &_output, const QuerySingletonToEdit &_task)
{
    return _output << "Query singleton from \"" << _task.sourceName << "\" and save editable cursor as \"" <<
                   _task.cursorName << "\".";
}

std::ostream &operator << (std::ostream &_output, const QueryUnorderedSequenceToRead &_task)
{
    return _output << "Query objects sequence from \"" << _task.sourceName << "\" and save read only cursor as \"" <<
                   _task.cursorName << "\".";
}

std::ostream &operator << (std::ostream &_output, const QueryUnorderedSequenceToEdit &_task)
{
    return _output << "Query objects sequence from \"" << _task.sourceName << "\" and save editable cursor as \"" <<
                   _task.cursorName << "\".";
}

std::ostream &operator << (std::ostream &_output, const QueryValueToRead &_task)
{
    return _output << "Query objects from \"" << _task.sourceName << "\" using value sequence " <<
                   _task.value << " and save read only cursor as \"" << _task.cursorName << "\".";
}

std::ostream &operator << (std::ostream &_output, const QueryValueToEdit &_task)
{
    return _output << "Query objects from \"" << _task.sourceName << "\" using value sequence " <<
                   _task.value << " and save editable cursor as \"" << _task.cursorName << "\".";
}

std::ostream &operator << (std::ostream &_output, const QueryRangeToRead &_task)
{
    return _output << "Query objects from \"" << _task.sourceName << "\" using range [" <<
                   _task.minValue << ", " << _task.maxValue << "] and save read only cursor as \"" <<
                   _task.cursorName << "\".";
}

std::ostream &operator << (std::ostream &_output, const QueryRangeToEdit &_task)
{
    return _output << "Query objects from \"" << _task.sourceName << "\" using range [" <<
                   _task.minValue << ", " << _task.maxValue << "] and save editable cursor as \"" <<
                   _task.cursorName << "\".";
}

std::ostream &operator << (std::ostream &_output, const QueryReversedRangeToRead &_task)
{
    return _output << "Query objects from \"" << _task.sourceName << "\" using range [" <<
                   _task.minValue << ", " << _task.maxValue << "] and save read only reversed order cursor as \"" <<
                   _task.cursorName << "\".";
}

std::ostream &operator << (std::ostream &_output, const QueryReversedRangeToEdit &_task)
{
    return _output << "Query objects from \"" << _task.sourceName << "\" using range [" <<
                   _task.minValue << ", " << _task.maxValue << "] and save editable reversed order cursor as \"" <<
                   _task.cursorName << "\".";
}

std::ostream &operator << (std::ostream &_output, const std::vector <Sources::Volumetric::SupportedValue> &_vector)
{
    for (auto iterator = _vector.begin (); iterator != _vector.end (); ++iterator)
    {
        _output << *iterator;
        if (iterator + 1u != _vector.end ())
        {
            _output << ", ";
        }
    }

    return _output << ")";
}

std::ostream &operator << (std::ostream &_output, const QueryShapeIntersectionToRead &_task)
{
    return _output << "Query objects from \"" << _task.sourceName << "\" using shape (min = (" <<
                   _task.min << "), max = (" << _task.max << ")) and save read only cursor as \"" <<
                   _task.cursorName << "\".";
}

std::ostream &operator << (std::ostream &_output, const QueryShapeIntersectionToEdit &_task)
{
    return _output << "Query objects from \"" << _task.sourceName << "\" using shape (min = " <<
                   _task.min << ", max = " << _task.max << ") and save editable cursor as \"" <<
                   _task.cursorName << "\".";
}

std::ostream &operator << (std::ostream &_output, const QueryRayIntersectionToRead &_task)
{
    return _output << "Query objects from \"" << _task.sourceName << "\" using ray (origin = " <<
                   _task.origin << ", direction = " << _task.direction << ") and save read only cursor as \"" <<
                   _task.cursorName << "\".";
}

std::ostream &operator << (std::ostream &_output, const QueryRayIntersectionToEdit &_task)
{
    return _output << "Query objects from \"" << _task.sourceName << "\" using ray (origin = " <<
                   _task.origin << ", direction = " << _task.direction << ") and save editable cursor as \"" <<
                   _task.cursorName << "\".";
}

std::ostream &operator << (std::ostream &_output, const CursorCheck &_task)
{
    return _output << "Check that cursor \"" << _task.name << "\" points to object, equal to " <<
                   _task.expectedObject << ".";
}

std::ostream &operator << (std::ostream &_output, const CursorCheckAllOrdered &_task)
{
    _output << "Check that cursor \"" << _task.name << "\" points to ordered sequence of objects equal to:";
    for (const void *object : _task.expectedObjects)
    {
        _output << " " << object;
    }

    return _output << ".";
}

std::ostream &operator << (std::ostream &_output, const CursorCheckAllUnordered &_task)
{
    _output << "Check that cursor \"" << _task.name << "\" points to set of objects equal to:";
    for (const void *object : _task.expectedObjects)
    {
        _output << " " << object;
    }

    return _output << ".";
}

std::ostream &operator << (std::ostream &_output, const CursorEdit &_task)
{
    return _output << "Replace value of object, to which cursor \"" << _task.name <<
                   "\" points with value of " << _task.copyFromObject << ".";
}

std::ostream &operator << (std::ostream &_output, const CursorIncrement &_task)
{
    return _output << "Increment cursor \"" << _task.name << "\".";
}

std::ostream &operator << (std::ostream &_output, const CursorDeleteObject &_task)
{
    return _output << "Delete object, to which cursor \"" << _task.name << "\" points.";
}

std::ostream &operator << (std::ostream &_output, const CursorClose &_task)
{
    return _output << "Close cursor \"" << _task.name << "\".";
}
} // namespace Tasks

Task ChangeQuerySourceAndCursor (
    Task _query, std::optional <std::string> _newSourceName, std::optional <std::string> _newCursorName)
{
    std::visit (
        [&_newSourceName, &_newCursorName] (auto &_task)
        {
            if constexpr (std::is_base_of_v <Tasks::QueryBase, std::decay_t <decltype (_task)>>)
            {
                if (_newSourceName.has_value ())
                {
                    _task.sourceName = std::move (_newSourceName.value ());
                }

                if (_newCursorName.has_value ())
                {
                    _task.cursorName = std::move (_newCursorName.value ());
                }
            }
            else
            {
                REQUIRE_WITH_MESSAGE (false, "Query must be derived from QueryBase!");
            }
        },
        _query);

    return _query;
}

Scenario RemapSources (Scenario _scenario, const std::unordered_map <std::string, std::string> &_transformation)
{
    for (Storage &storage : _scenario.storages)
    {
        for (Source &source : storage.sources)
        {
            std::visit (
                [&_transformation] (auto &_source)
                {
                    auto newNameIterator = _transformation.find (_source.name);
                    if (newNameIterator != _transformation.end ())
                    {
                        _source.name = newNameIterator->second;
                    }
                },
                source);
        }
    }

    for (Task &task : _scenario.tasks)
    {
        std::visit (
            [&_transformation] (auto &_task)
            {
                if constexpr (std::is_base_of_v <Tasks::QueryBase, std::decay_t <decltype (_task)>>)
                {
                    auto newNameIterator = _transformation.find (_task.sourceName);
                    if (newNameIterator != _transformation.end ())
                    {
                        _task.sourceName = newNameIterator->second;
                    }
                }
            },
            task);
    }

    return _scenario;
}

std::vector <Task> &operator += (std::vector <Task> &_left, const std::vector <Task> &_right)
{
    _left.insert (_left.end (), _right.begin (), _right.end ());
    return _left;
}

std::ostream &operator << (std::ostream &_output, const Task &_task)
{
    return std::visit (
        [&_output] (const auto &_unwrappedTask) -> std::ostream &
        {
            return _output << _unwrappedTask;
        },
        _task);
}
} // namespace Emergence::Query::Test