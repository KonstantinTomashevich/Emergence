#include <RecordCollection/Test/Common.hpp>

namespace Emergence::RecordCollection::Test::Common
{
std::vector <Task> TestIsCanBeDropped (const std::string &_representationName)
{
    return
        {
            CheckIsSourceBusy {_representationName, false},
            CopyRepresentationReference {_representationName, _representationName + "_duplicate"},
            CheckIsSourceBusy {_representationName, true},
            CheckIsSourceBusy {_representationName + "_duplicate", true},
            RemoveRepresentationReference {_representationName},
            CheckIsSourceBusy {_representationName + "_duplicate", false},
            DropRepresentation {_representationName + "_duplicate"},
        };
}
} // namespace Emergence::RecordCollection::Test::Common

