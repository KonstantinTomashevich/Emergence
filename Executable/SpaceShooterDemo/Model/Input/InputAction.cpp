#include <Input/InputAction.hpp>

#include <StandardLayout/MappingRegistration.hpp>

InputAction::InputAction (const Emergence::Memory::UniqueString &_group,
                          const Emergence::Memory::UniqueString &_id) noexcept
    : group (_group),
      id (_id)
{
}

InputAction::InputAction (const Emergence::Memory::UniqueString &_group,
                          const Emergence::Memory::UniqueString &_id,
                          const std::array<int32_t, MAX_DISCRETE_PARAMETERS> &_discrete) noexcept
    : group (_group),
      id (_id)
{
    discrete = _discrete;
}

InputAction::InputAction (const Emergence::Memory::UniqueString &_group,
                          const Emergence::Memory::UniqueString &_id,
                          const std::array<float, MAX_REAL_PARAMETERS> &_real) noexcept
    : group (_group),
      id (_id)
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
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (InputAction);
        EMERGENCE_MAPPING_REGISTER_REGULAR (group);
        EMERGENCE_MAPPING_REGISTER_REGULAR (id);
        EMERGENCE_MAPPING_REGISTER_REGULAR_ARRAY (discrete);
        EMERGENCE_MAPPING_REGISTER_REGULAR_ARRAY (real);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
