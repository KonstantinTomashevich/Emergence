#pragma once

#include <API/Common/Cursor.hpp>
#include <API/Common/Shortcuts.hpp>

#include <Galleon/ContainerBase.hpp>

namespace Emergence::Galleon
{
class LongTermContainer final : public ContainerBase
    {
    public:

    private:
        /// CargoDeck constructs containers.
        friend class CargoDeck;

    /// Only handles have right to destruct containers.
    template <typename>
    friend class Handling::Handle;

    /// \warning Must be used in pair with custom ::new.
    explicit LongTermContainer (CargoDeck *_deck, StandardLayout::Mapping _typeMapping) noexcept;

    /// \warning Must be used in pair with custom ::delete.
    ~LongTermContainer () noexcept;
    };
} // namespace Emergence::Galleon
