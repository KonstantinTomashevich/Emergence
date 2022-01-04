#include <Input/InputMapping.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const KeyRequirement::Reflection &KeyRequirement::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (KeyRequirement)
        EMERGENCE_MAPPING_REGISTER_REGULAR (key)
        EMERGENCE_MAPPING_REGISTER_REGULAR (justPressed)
        EMERGENCE_MAPPING_REGISTER_ENUM_AS_REGULAR (currentKeyState)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}

const KeyboardActionTrigger::Reflection &KeyboardActionTrigger::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (KeyboardActionTrigger)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (action)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT_ARRAY (requirements)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}

const InputSubscription::Reflection &InputSubscription::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (InputSubscription)
        EMERGENCE_MAPPING_REGISTER_BLOCK (group)
        EMERGENCE_MAPPING_REGISTER_REGULAR (listenerId)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}

void InputMapping::UnsubscribeListener (std::uintptr_t _listenerId)
{
    auto iterator = subscriptions.Begin ();
    while (iterator != subscriptions.End ())
    {
        if (iterator->listenerId == _listenerId)
        {
            iterator = subscriptions.EraseExchangingWithLast (iterator);
        }
        else
        {
            ++iterator;
        }
    }
}

void InputMapping::UnsubscribeGroup (Emergence::Memory::UniqueString _group)
{
    auto iterator = subscriptions.Begin ();
    while (iterator != subscriptions.End ())
    {
        if (iterator->group == _group)
        {
            iterator = subscriptions.EraseExchangingWithLast (iterator);
        }
        else
        {
            ++iterator;
        }
    }
}

const InputMapping::Reflection &InputMapping::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (InputMapping)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT_ARRAY (keyboardTriggers)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT_ARRAY (subscriptions)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}
