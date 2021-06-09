#pragma once

#include <Handling/HandleableBase.hpp>

namespace Emergence::Pegasus
{
// TODO: Add self reference to prevent Handling from deleting objects.
class OrderedIndex final : public Handling::HandleableBase
{
};
} // namespace Emergence::Pegasus