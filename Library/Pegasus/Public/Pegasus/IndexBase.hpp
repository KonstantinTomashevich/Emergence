#pragma once

#include <atomic>

#include <Handling/HandleableBase.hpp>

namespace Emergence::Pegasus
{
/// \brief Minimal base class for all Pegasus indices.
class IndexBase : public Handling::HandleableBase
{
public:
    bool CanBeDropped () const noexcept;

protected:
    explicit IndexBase (class Storage *_storage) noexcept;

    class Storage *storage;
    std::atomic_size_t activeCursors;
};
} // namespace Emergence::Pegasus
