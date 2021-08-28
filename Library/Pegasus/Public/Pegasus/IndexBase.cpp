#include <Pegasus/IndexBase.hpp>
#include <Pegasus/Storage.hpp>

namespace Emergence::Pegasus
{
bool IndexBase::CanBeDropped () const noexcept
{
    // Self reference is always here.
    return GetReferenceCount () <= 1u && activeCursors == 0u;
}

IndexBase::IndexBase (class Storage *_storage) noexcept : storage (_storage), activeCursors (0u)
{
    assert (storage);
    // Add self reference to prevent Handling from deleting this object.
    // Storage owns indices and Handling is used only to check if dropping index is safe.
    RegisterReference ();
}
} // namespace Emergence::Pegasus
