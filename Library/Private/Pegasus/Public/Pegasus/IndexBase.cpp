#include <Pegasus/IndexBase.hpp>
#include <Pegasus/Storage.hpp>

namespace Emergence::Pegasus
{
bool IndexBase::CanBeDropped () const noexcept
{
    return GetReferenceCount () == 0u && activeCursors == 0u;
}

const StandardLayout::Mapping &IndexBase::GetRecordMapping () const noexcept
{
    return storage->GetRecordMapping ();
}

IndexBase::IndexBase (class Storage *_storage) noexcept : storage (_storage), activeCursors (0u)
{
    assert (storage);
}

void IndexBase::LastReferenceUnregistered () noexcept
{
    // Do nothing, we're just counting references to determinate if index could be dropped.
}
} // namespace Emergence::Pegasus
