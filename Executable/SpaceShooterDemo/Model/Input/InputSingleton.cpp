#include <Input/InputSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const KeyStateTrigger::Reflection &KeyStateTrigger::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (KeyStateTrigger);
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (action);
        EMERGENCE_MAPPING_REGISTER_REGULAR (scan);
        EMERGENCE_MAPPING_REGISTER_REGULAR (down);
        EMERGENCE_MAPPING_REGISTER_REGULAR (isDownNow);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const KeyStateChangedTrigger::Reflection &KeyStateChangedTrigger::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (KeyStateChangedTrigger);
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (action);
        EMERGENCE_MAPPING_REGISTER_REGULAR (scan);
        EMERGENCE_MAPPING_REGISTER_REGULAR (pressed);
        EMERGENCE_MAPPING_REGISTER_REGULAR (qualifiers);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const InputSubscription::Reflection &InputSubscription::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (InputSubscription);
        EMERGENCE_MAPPING_REGISTER_UNIQUE_STRING (group);
        EMERGENCE_MAPPING_REGISTER_REGULAR (listenerId);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

static void UnsubscribeListener (InputSingleton::SubscriptionVector &_subscriptions,
                                 std::uintptr_t _listenerId) noexcept
{
    auto iterator = _subscriptions.Begin ();
    while (iterator != _subscriptions.End ())
    {
        if (iterator->listenerId == _listenerId)
        {
            iterator = _subscriptions.EraseExchangingWithLast (iterator);
        }
        else
        {
            ++iterator;
        }
    }
}

void InputSingleton::UnsubscribeNormal (std::uintptr_t _listenerId) noexcept
{
    UnsubscribeListener (normalSubscriptions, _listenerId);
}

void InputSingleton::UnsubscribeFixed (std::uintptr_t _listenerId) noexcept
{
    UnsubscribeListener (fixedSubscriptions, _listenerId);
}

void InputSingleton::UnsubscribeGroup (Emergence::Memory::UniqueString _group) noexcept
{
    auto unsubscribe = [_group] (SubscriptionVector &_subscriptions)
    {
        auto iterator = _subscriptions.Begin ();
        while (iterator != _subscriptions.End ())
        {
            if (iterator->group == _group)
            {
                iterator = _subscriptions.EraseExchangingWithLast (iterator);
            }
            else
            {
                ++iterator;
            }
        }
    };

    unsubscribe (normalSubscriptions);
    unsubscribe (fixedSubscriptions);
}

const InputSingleton::Reflection &InputSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (InputSingleton);
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT_ARRAY (keyStateTriggers);
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT_ARRAY (keyStateChangedTriggers);
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT_ARRAY (normalSubscriptions);
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT_ARRAY (fixedSubscriptions);
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT_ARRAY (normalActionsBuffer);
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT_ARRAY (fixedInstantActionsBuffer);
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT_ARRAY (fixedPersistentActionsBuffer);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
