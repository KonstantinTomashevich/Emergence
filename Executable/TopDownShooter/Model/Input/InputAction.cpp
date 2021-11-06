#include <Input/InputAction.hpp>

#include <SyntaxSugar/MappingRegistration.hpp>

InputAction::InputAction (uint64_t _id) noexcept : id (_id)
{
}

InputAction::InputAction (uint64_t _id, const std::array<int32_t, MAX_DISCRETE_PARAMETERS> &_discrete) noexcept
    : id (_id)
{
    discrete = _discrete;
}

InputAction::InputAction (uint64_t _id, const std::array<float, MAX_REAL_PARAMETERS> &_real) noexcept : id (_id)
{
    real = _real;
}

const InputAction::Reflection &InputAction::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (InputAction)
        EMERGENCE_MAPPING_REGISTER_REGULAR (id)
        EMERGENCE_MAPPING_REGISTER_REGULAR_ARRAY (discrete)
        EMERGENCE_MAPPING_REGISTER_REGULAR_ARRAY (real)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}
