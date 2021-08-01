#include <cassert>

#include <Galleon/CargoDeck.hpp>
#include <Galleon/LongTermContainer.hpp>

namespace Emergence::Galleon
{
LongTermContainer::LongTermContainer (CargoDeck *_deck, StandardLayout::Mapping _typeMapping) noexcept
    : ContainerBase (_deck, std::move (_typeMapping))
{

}

LongTermContainer::~LongTermContainer () noexcept
{
    assert (deck);
    deck->DetachContainer (this);
}
} // namespace Emergence::Galleon