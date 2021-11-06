#include <cassert>

#include <OgreRoot.h>
#include <OgreTimer.h>

#include <Input/InputAccumulator.hpp>

InputAccumulator::InputAccumulator (OgreBites::ApplicationContext *_application,
                                    const std::vector<KeyboardActionTrigger> &_keyboardTriggers) noexcept
    : application (_application)
{
    assert (application);
    application->addInputListener (this);

    for (const KeyboardActionTrigger &trigger : _keyboardTriggers)
    {
        KeyboardActionTriggerState &triggerState = keyboardTriggers.emplace_back ();
        triggerState.action = trigger.action;

        for (const KeyRequirement &requirement : trigger.requirements)
        {
            KeyRequirementState &requirementState = triggerState.requirementStates.EmplaceBack ();
            requirementState.requirement = requirement;
        }
    }
}

InputAccumulator::~InputAccumulator () noexcept
{
    application->removeInputListener (this);
}

bool InputAccumulator::PopNextAction (InputAction &_output, uint64_t _tillMs) noexcept
{
    if (triggeredActions.empty () || triggeredActions.front ().triggeredAtMs > _tillMs)
    {
        return false;
    }

    _output = triggeredActions.front ().action;
    triggeredActions.pop ();
    return true;
}

bool InputAccumulator::keyPressed (const OgreBites::KeyboardEvent &_event)
{
    bool anySatisfied = false;
    Ogre::Timer *timer = application->getRoot ()->getTimer ();
    uint64_t currentTimeMs = timer->getMilliseconds ();
    assert (triggeredActions.empty () || triggeredActions.back ().triggeredAtMs <= currentTimeMs);

    for (KeyboardActionTriggerState &triggerState : keyboardTriggers)
    {
        bool satisfied = true;
        for (KeyRequirementState &requirementState : triggerState.requirementStates)
        {
            if (requirementState.requirement.key == _event.keysym.sym)
            {
                switch (requirementState.keyCurrentState)
                {
                case KeyState::JUST_PRESSED:
                    requirementState.keyCurrentState = KeyState::PRESSED;
                    break;
                case KeyState::PRESSED:
                    break;
                case KeyState::RELEASED:
                    requirementState.keyCurrentState = KeyState::JUST_PRESSED;
                    break;
                }
            }

            if (requirementState.requirement.justPressed)
            {
                satisfied &= requirementState.keyCurrentState == KeyState::JUST_PRESSED;
            }
            else
            {
                satisfied &= requirementState.keyCurrentState == KeyState::JUST_PRESSED ||
                             requirementState.keyCurrentState == KeyState::PRESSED;
            }
        }

        if (satisfied)
        {
            anySatisfied = true;
            triggeredActions.emplace (TriggeredAction {triggerState.action, currentTimeMs});
        }
    }

    return anySatisfied;
}

bool InputAccumulator::keyReleased (const OgreBites::KeyboardEvent &_event)
{
    for (KeyboardActionTriggerState &triggerState : keyboardTriggers)
    {
        for (KeyRequirementState &requirementState : triggerState.requirementStates)
        {
            if (requirementState.requirement.key == _event.keysym.sym)
            {
                requirementState.keyCurrentState = KeyState::RELEASED;
            }
        }
    }

    return false;
}
