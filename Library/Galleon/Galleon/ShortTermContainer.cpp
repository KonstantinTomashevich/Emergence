#include <cassert>

#include <Galleon/CargoDeck.hpp>
#include <Galleon/ShortTermContainer.hpp>

namespace Emergence::Galleon
{
ShortTermContainer::ShortTermContainer (CargoDeck *_deck, StandardLayout::Mapping _typeMapping) noexcept
    : ContainerBase (_deck, std::move (_typeMapping))
{

}

ShortTermContainer::~ShortTermContainer () noexcept
{
    assert (deck);
    deck->DetachContainer (this);
}
} // namespace Emergence::Galleon