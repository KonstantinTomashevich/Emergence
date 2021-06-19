#pragma once

#include <atomic>

#include <Handling/HandleableBase.hpp>

namespace Emergence::Pegasus
{
class IndexBase : public Handling::HandleableBase
{
public:
    bool CanBeDropped () const noexcept;

protected:
    explicit IndexBase (class Storage *_storage) noexcept;

    class Storage *storage;
    std::atomic_size_t activeCursors = 0u;
};
} // namespace Emergence::Pegasus
