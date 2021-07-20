#include <iostream>

#include <Query/Test/Scenario.hpp>

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
std::ostream &operator << (std::ostream &_output, const Tasks::CheckIsSourceBusy &_task)
{
    return _output << "Check is source \"" << _task.name << "\" busy, expected result: \"" <<
                   (_task.expectedValue ? "yes" : "no") << "\".";
}

std::ostream &operator << (std::ostream &_output, const Tasks::QueryValueToRead &_task)
{
    return _output << "Query objects from \"" << _task.sourceName << "\" using value sequence " <<
                   _task.value << " and save read only cursor as \"" << _task.cursorName << "\".";
}

std::ostream &operator << (std::ostream &_output, const Tasks::QueryValueToEdit &_task)
{
    return _output << "Query objects from \"" << _task.sourceName << "\" using value sequence " <<
                   _task.value << " and save editable cursor as \"" << _task.cursorName << "\".";
}

std::ostream &operator << (std::ostream &_output, const Tasks::QueryRangeToRead &_task)
{
    return _output << "Query objects from \"" << _task.sourceName << "\" using range [" <<
                   _task.minValue << ", " << _task.maxValue << "] and save read only cursor as \"" <<
                   _task.cursorName << "\".";
}

std::ostream &operator << (std::ostream &_output, const Tasks::QueryRangeToEdit &_task)
{
    return _output << "Query objects from \"" << _task.sourceName << "\" using range [" <<
                   _task.minValue << ", " << _task.maxValue << "] and save editable cursor as \"" <<
                   _task.cursorName << "\".";
}

std::ostream &operator << (std::ostream &_output, const Tasks::QueryReversedRangeToRead &_task)
{
    return _output << "Query objects from \"" << _task.sourceName << "\" using range [" <<
                   _task.minValue << ", " << _task.maxValue << "] and save read only reversed order cursor as \"" <<
                   _task.cursorName << "\".";
}

std::ostream &operator << (std::ostream &_output, const Tasks::QueryReversedRangeToEdit &_task)
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

std::ostream &operator << (std::ostream &_output, const Tasks::QueryShapeIntersectionToRead &_task)
{
    return _output << "Query objects from \"" << _task.sourceName << "\" using shape (min = (" <<
                   _task.min << "), max = (" << _task.max << ")) and save read only cursor as \"" <<
                   _task.cursorName << "\".";
}

std::ostream &operator << (std::ostream &_output, const Tasks::QueryShapeIntersectionToEdit &_task)
{
    return _output << "Query objects from \"" << _task.sourceName << "\" using shape (min = " <<
                   _task.min << ", max = " << _task.max << ") and save editable cursor as \"" <<
                   _task.cursorName << "\".";
}

std::ostream &operator << (std::ostream &_output, const Tasks::QueryRayIntersectionToRead &_task)
{
    return _output << "Query objects from \"" << _task.sourceName << "\" using ray (origin = " <<
                   _task.origin << ", direction = " << _task.direction << ") and save read only cursor as \"" <<
                   _task.cursorName << "\".";
}

std::ostream &operator << (std::ostream &_output, const Tasks::QueryRayIntersectionToEdit &_task)
{
    return _output << "Query objects from \"" << _task.sourceName << "\" using ray (origin = " <<
                   _task.origin << ", direction = " << _task.direction << ") and save editable cursor as \"" <<
                   _task.cursorName << "\".";
}

std::ostream &operator << (std::ostream &_output, const Tasks::CursorCheck &_task)
{
    return _output << "Check that cursor \"" << _task.name << "\" points to object, equal to " <<
                   _task.expectedObject << ".";
}

std::ostream &operator << (std::ostream &_output, const Tasks::CursorCheckAllOrdered &_task)
{
    _output << "Check that cursor \"" << _task.name << "\" points to ordered sequence of objects equal to:";
    for (const void *record : _task.expectedObjects)
    {
        _output << " " << record;
    }

    return _output << ".";
}

std::ostream &operator << (std::ostream &_output, const Tasks::CursorCheckAllUnordered &_task)
{
    _output << "Check that cursor \"" << _task.name << "\" points to set of objects equal to:";
    for (const void *record : _task.expectedObjects)
    {
        _output << " " << record;
    }

    return _output << ".";
}

std::ostream &operator << (std::ostream &_output, const Tasks::CursorEdit &_task)
{
    return _output << "Replace value of object, to which cursor \"" << _task.name <<
                   "\" points with value of " << _task.copyFromObject << ".";
}

std::ostream &operator << (std::ostream &_output, const Tasks::CursorIncrement &_task)
{
    return _output << "Increment cursor \"" << _task.name << "\".";
}

std::ostream &operator << (std::ostream &_output, const Tasks::CursorDeleteRecord &_task)
{
    return _output << "Delete object, to which cursor \"" << _task.name << "\" points.";
}

std::ostream &operator << (std::ostream &_output, const Tasks::CursorCopy &_task)
{
    return _output << "Copy cursor \"" << _task.sourceName << "\" as \"" << _task.targetName << ".";
}

std::ostream &operator << (std::ostream &_output, const Tasks::CursorMove &_task)
{
    return _output << "Move cursor \"" << _task.sourceName << "\" to \"" << _task.targetName << ".";
}

std::ostream &operator << (std::ostream &_output, const Tasks::CursorClose &_task)
{
    return _output << "Close cursor \"" << _task.name << "\".";
}
} // namespace Tasks

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