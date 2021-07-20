#include <Pegasus/Test/Common.hpp>

namespace Emergence::Pegasus::Test::Shortcuts
{
std::vector <Task> TestIsCanBeDropped (const std::string &_indexName)
{
    return
        {
            CheckIsSourceBusy {_indexName, false},
            CopyIndexReference {_indexName, _indexName + "_duplicate"},
            CheckIsSourceBusy {_indexName, true},
            CheckIsSourceBusy {_indexName + "_duplicate", true},
            RemoveIndexReference {_indexName},
            CheckIsSourceBusy {_indexName + "_duplicate", false},
            DropIndex {_indexName + "_duplicate"},
        };
}
} // namespace Emergence::Pegasus::Test::Common
