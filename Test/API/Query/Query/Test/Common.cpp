#include <cassert>

#include <Query/Test/Common.hpp>

namespace Emergence::Query::Test
{
static std::pair <std::string, std::string> ExtractSourceAndCursorNames (const Task &_task)
{
    using namespace Tasks;
    return std::visit (
        [] (const auto &_unwrappedTask) -> std::pair <std::string, std::string>
        {
            using TaskType = std::decay_t <decltype (_unwrappedTask)>;
            if constexpr (std::is_convertible_v <TaskType, QueryBase>)
            {
                const QueryBase &query = static_cast <const QueryBase &> (_unwrappedTask);
                return {query.sourceName, query.cursorName};
            }
            else
            {
                assert (false);
                return
                    {
                        "Error: QueryBase derivative expected!",
                        "Error: QueryBase derivative expected!",
                    };
            }
        },
        _task);
}

std::vector <Task> TestCursorCopyAndMove (
    const Task &_readCursorQuery, const Task &_editCursorQuery, const void *_readCursorExpectedFirstObject,
    const void *_readCursorExpectedSecondObject, const void *_editCursorExpectedFirstObject)
{
    using namespace Tasks;
    auto[sourceName, readCursorName] = ExtractSourceAndCursorNames (_readCursorQuery);
    auto[sourceNameFromEditQuery, editCursorName] = ExtractSourceAndCursorNames (_editCursorQuery);
    assert (sourceName == sourceNameFromEditQuery);

    return
        {
            CheckIsSourceBusy {sourceName, false},
            _readCursorQuery,
            CheckIsSourceBusy {sourceName, true},

            CursorCheck {readCursorName, _readCursorExpectedFirstObject},
            CursorCopy {readCursorName, readCursorName + "_copy"},
            CheckIsSourceBusy {sourceName, true},

            CursorIncrement {readCursorName},
            CursorCheck {readCursorName, _readCursorExpectedSecondObject},
            CursorCheck {readCursorName + "_copy", _readCursorExpectedFirstObject},

            CursorMove {readCursorName + "_copy", readCursorName + "_move"},
            CheckIsSourceBusy {sourceName, true},
            CursorCheck {readCursorName + "_move", _readCursorExpectedFirstObject},

            CursorClose {readCursorName},
            CheckIsSourceBusy {sourceName, true},
            CursorClose {readCursorName + "_move"},
            CheckIsSourceBusy {sourceName, false},

            _editCursorQuery,
            CheckIsSourceBusy {sourceName, true},

            CursorCheck {editCursorName, _editCursorExpectedFirstObject},
            CursorMove {editCursorName, editCursorName + "_move"},
            CheckIsSourceBusy {sourceName, true},

            CursorCheck {editCursorName + "_move", _editCursorExpectedFirstObject},
            CursorClose {editCursorName + "_move"},
            CheckIsSourceBusy {sourceName, false},
        };
}

} // namespace Emergence::Query::Test