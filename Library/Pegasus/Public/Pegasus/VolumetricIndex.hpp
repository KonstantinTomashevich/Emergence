#pragma once

#include <Handling/HandleableBase.hpp>

#include <Pegasus/Constants/VolumetricIndex.hpp>

namespace Emergence::Pegasus
{
// TODO: Add self reference to prevent Handling from deleting objects.
class VolumetricIndex final : public Handling::HandleableBase
{
};
} // namespace Emergence::Pegasus