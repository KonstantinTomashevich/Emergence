#include <cassert>

#include <Pegasus/Test/Shortcuts.hpp>

namespace Emergence::Pegasus::Test::Shortcuts
{
std::vector <Task> TestIsCanBeDropped (const std::string &_indexName)
{
    return
        {
            CheckIndexCanBeDropped {_indexName, true},
            CopyIndexReference {_indexName, _indexName + "_duplicate"},
            CheckIndexCanBeDropped {_indexName, false},
            CheckIndexCanBeDropped {_indexName + "_duplicate", false},
            RemoveIndexReference {_indexName},
            CheckIndexCanBeDropped {_indexName + "_duplicate", true},
            DropIndex {_indexName + "_duplicate"},
        };
}

static IndexLookupBase ExtractLookupBaseData (const Task &_task)
{
    return std::visit (
        [] (const auto &_unwrappedTask) -> IndexLookupBase
        {
            using TaskType = std::decay_t <decltype (_unwrappedTask)>;
            if constexpr (std::is_convertible_v <TaskType, IndexLookupBase>)
            {
                return static_cast<const IndexLookupBase &>(_unwrappedTask);
            }
            else
            {
                assert (false);
                return
                    {
                        "Error: IndexLookupBase derivative expected!",
                        "Error: IndexLookupBase derivative expected!"
                    };
            }
        },
        _task);
}

std::vector <Task> TestCursorCopyAndMove (
    const Task &_readCursorConstructor, const Task &_editCursorConstructor,
    const void *_readCursorExpectedFirstRecord, const void *_readCursorExpectedSecondRecord,
    const void *_editCursorExpectedFirstRecord)
{
    auto[indexName, readCursorName] = ExtractLookupBaseData (_readCursorConstructor);
    auto[anotherIndexName, editCursorName] = ExtractLookupBaseData (_editCursorConstructor);
    assert (indexName == anotherIndexName);

    return
        {
            CheckIndexCanBeDropped {indexName, true},
            _readCursorConstructor,
            CheckIndexCanBeDropped {indexName, false},

            CursorCheck {readCursorName, _readCursorExpectedFirstRecord},
            CopyCursor {readCursorName, readCursorName + "_copy"},

            CursorIncrement {readCursorName},
            CursorCheck {readCursorName, _readCursorExpectedSecondRecord},
            CursorCheck {readCursorName + "_copy", _readCursorExpectedFirstRecord},

            MoveCursor {readCursorName + "_copy", readCursorName + "_move"},
            CursorCheck {readCursorName + "_move", _readCursorExpectedFirstRecord},
            CheckIndexCanBeDropped {indexName, false},

            CloseCursor {readCursorName},
            CheckIndexCanBeDropped {indexName, false},
            CloseCursor {readCursorName + "_move"},
            CheckIndexCanBeDropped {indexName, true},

            _editCursorConstructor,
            CheckIndexCanBeDropped {indexName, false},
            CursorCheck {editCursorName, _editCursorExpectedFirstRecord},
            MoveCursor {editCursorName, editCursorName + "_move"},
            CheckIndexCanBeDropped {indexName, false},

            CursorCheck {editCursorName + "_move", _editCursorExpectedFirstRecord},
            CloseCursor {editCursorName + "_move"},
            CheckIndexCanBeDropped {indexName, true},
        };
}
} // namespace Emergence::Pegasus::Test::Common

