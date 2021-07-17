#include <cassert>

#include <Query/Test/Common.hpp>

namespace Emergence::Query::Test
{
static std::string ExtractCursorName (const Task &_task)
{
    using namespace Tasks;
    return std::visit (
        [] (const auto &_unwrappedTask) -> std::string
        {
            using TaskType = std::decay_t <decltype (_unwrappedTask)>;
            if constexpr (std::is_convertible_v <TaskType, QueryBase>)
            {
                return static_cast <const QueryBase &> (_unwrappedTask).cursorName;
            }
            else
            {
                assert (false);
                return "Error: QueryBase derivative expected!";
            }
        },
        _task);
}

std::vector <Task> TestCursorCopyAndMove (
    const Task &_readCursorQuery, const Task &_editCursorQuery, const void *_readCursorExpectedFirstObject,
    const void *_readCursorExpectedSecondObject, const void *_editCursorExpectedFirstObject)
{
    using namespace Tasks;
    std::string readCursorName = ExtractCursorName (_readCursorQuery);
    std::string editCursorName = ExtractCursorName (_editCursorQuery);

    return
        {
            _readCursorQuery,
            CursorCheck {readCursorName, _readCursorExpectedFirstObject},
            CursorCopy {readCursorName, readCursorName + "_copy"},

            CursorIncrement {readCursorName},
            CursorCheck {readCursorName, _readCursorExpectedSecondObject},
            CursorCheck {readCursorName + "_copy", _readCursorExpectedFirstObject},

            CursorCopy {readCursorName + "_copy", readCursorName + "_move"},
            CursorCheck {readCursorName + "_move", _readCursorExpectedFirstObject},

            CursorClose {readCursorName},
            CursorClose {readCursorName + "_move"},

            _editCursorQuery,
            CursorCheck {editCursorName, _editCursorExpectedFirstObject},
            CursorMove {editCursorName, editCursorName + "_move"},

            CursorCheck {editCursorName + "_move", _editCursorExpectedFirstObject},
            CursorClose {editCursorName + "_move"},
        };
}
} // namespace Emergence::Query::Test