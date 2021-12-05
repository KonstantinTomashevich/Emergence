#include <Input/InputAction.hpp>

#include <SyntaxSugar/MappingRegistration.hpp>

InputAction::InputAction (const Emergence::String::ConstReference &_id,
                          const Emergence::String::ConstReference &_group) noexcept
    : id (_id),
      group (_group)
{
}

InputAction::InputAction (const Emergence::String::ConstReference &_id,
                          const Emergence::String::ConstReference &_group,
                          const std::array<int32_t, MAX_DISCRETE_PARAMETERS> &_discrete) noexcept
    : id (_id),
      group (_group)
{
    discrete = _discrete;
}

InputAction::InputAction (const Emergence::String::ConstReference &_id,
                          const Emergence::String::ConstReference &_group,
                          const std::array<float, MAX_REAL_PARAMETERS> &_real) noexcept
    : id (_id),
      group (_group)
{
    real = _real;
}

bool InputAction::operator== (const InputAction &_other) const
{
    return id == _other.id && group == _other.group &&
           // We check byte-to-byte equality of action parameters by checking their discrete representation.
           discrete == _other.discrete;
}

bool InputAction::operator!= (const InputAction &_other) const
{
    return !(_other == *this);
}

const InputAction::Reflection &InputAction::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (InputAction)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (id)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (group)
        EMERGENCE_MAPPING_REGISTER_REGULAR_ARRAY (discrete)
        EMERGENCE_MAPPING_REGISTER_REGULAR_ARRAY (real)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}