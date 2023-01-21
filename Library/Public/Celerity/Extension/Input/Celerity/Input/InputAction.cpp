#include <Celerity/Input/InputAction.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
InputAction::InputAction () noexcept
{
    // Fill everything with zero's for stable equality check.
    discrete.fill (0);
}

InputAction::InputAction (const Memory::UniqueString &_group, const Memory::UniqueString &_id) noexcept
    : group (_group),
      id (_id)
{
    // Fill everything with zero's for stable equality check.
    discrete.fill (0);
}

InputAction::InputAction (const Memory::UniqueString &_group,
                          const Memory::UniqueString &_id,
                          const std::array<int32_t, MAX_DISCRETE_PARAMETERS> &_discrete) noexcept
    : group (_group),
      id (_id)
{
    discrete = _discrete;
}

InputAction::InputAction (const Memory::UniqueString &_group,
                          const Memory::UniqueString &_id,
                          const std::array<float, MAX_REAL_PARAMETERS> &_real) noexcept
    : group (_group),
      id (_id)
{
    real = _real;
}

InputAction::InputAction (const InputAction &_other) noexcept
    : group (_other.group),
      id (_other.id),
      discrete (_other.discrete)
{
}

InputAction::InputAction (InputAction &&_other) noexcept
    : group (_other.group),
      id (_other.id),
      discrete (_other.discrete)
{
}

InputAction &InputAction::operator= (const InputAction &_other) noexcept
{
    group = _other.group;
    id = _other.id;
    discrete = _other.discrete;
    return *this;
}

InputAction &InputAction::operator= (InputAction &&_other) noexcept
{
    group = _other.group;
    id = _other.id;
    discrete = _other.discrete;
    return *this;
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
} // namespace Emergence::Celerity
