#pragma once

#include <Handling/HandleableBase.hpp>

#include <Pegasus/Constants/VolumetricIndex.hpp>
#include <Pegasus/IndexBase.hpp>

namespace Emergence::Pegasus
{
class VolumetricIndex final : public IndexBase
{
public:
    void Drop () noexcept;
};
} // namespace Emergence::Pegasus