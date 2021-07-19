#include <Query/Test/Scenario.hpp>

namespace Emergence::Query::Test
{
Sources::Volumetric::SupportedValue::SupportedValue (int8_t _value) noexcept
    : int8 (_value)
{
}

Sources::Volumetric::SupportedValue::SupportedValue (int16_t _value) noexcept
    : int16 (_value)
{
}

Sources::Volumetric::SupportedValue::SupportedValue (int32_t _value) noexcept
    : int32 (_value)
{
}

Sources::Volumetric::SupportedValue::SupportedValue (int64_t _value) noexcept
    : int64 (_value)
{
}

Sources::Volumetric::SupportedValue::SupportedValue (uint8_t _value) noexcept
    : uint8 (_value)
{
}

Sources::Volumetric::SupportedValue::SupportedValue (uint16_t _value) noexcept
    : uint16 (_value)
{
}

Sources::Volumetric::SupportedValue::SupportedValue (uint32_t _value) noexcept
    : uint32 (_value)
{
}

Sources::Volumetric::SupportedValue::SupportedValue (uint64_t _value) noexcept
    : uint64 (_value)
{
}

Sources::Volumetric::SupportedValue::SupportedValue (float _value) noexcept
    : floating (_value)
{
}

Sources::Volumetric::SupportedValue::SupportedValue (double _value) noexcept
    : doubleFloating (_value)
{
}

std::vector <Task> &operator += (std::vector <Task> &_left, const std::vector <Task> &_right)
{
    _left.insert (_left.end (), _right.begin (), _right.end ());
    return _left;
}
} // namespace Emergence::Query::Test