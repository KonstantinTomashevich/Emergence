#include <cassert>

#include <OgreInput.h>

#include <Input/FixedInputMappingSingleton.hpp>
#include <Input/InputCollection.hpp>
#include <Input/InputListenerObject.hpp>
#include <Input/NormalInputMappingSingleton.hpp>

#include <Shared/Checkpoint.hpp>

namespace InputCollection
{
class InputCollector final : public OgreBites::InputListener
{
public:
    InputCollector (OgreBites::ApplicationContext *_application,
                    const Emergence::StandardLayout::Mapping &_singleton,
                    Emergence::StandardLayout::FieldId _mappingField,
                    Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    InputCollector (const InputCollector &_other) = delete;

    InputCollector (InputCollector &&_other) = delete;

    ~InputCollector () noexcept;

    void Execute () noexcept;

    bool keyPressed (const OgreBites::KeyboardEvent &_event) override;

    bool keyReleased (const OgreBites::KeyboardEvent &_event) override;

    EMERGENCE_DELETE_ASSIGNMENT (InputCollector);

private:
    Emergence::Warehouse::ModifySingletonQuery modifySingleton;
    Emergence::Warehouse::ModifyValueQuery modifyListenersById;

    OgreBites::ApplicationContext *application;
    Emergence::StandardLayout::Mapping singleton;
    Emergence::StandardLayout::Field mappingField;
};

InputCollector::InputCollector (OgreBites::ApplicationContext *_application,
                                const Emergence::StandardLayout::Mapping &_singleton,
                                Emergence::StandardLayout::FieldId _mappingField,
                                Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : modifySingleton (_constructor.ModifySingleton (_singleton)),
      modifyListenersById (_constructor.ModifyValue (InputListenerObject::Reflect ().mapping,
                                                     {InputListenerObject::Reflect ().objectId})),
      application (_application),
      singleton (_singleton),
      mappingField (_singleton.GetField (_mappingField))
{
    assert (_application);
    assert (mappingField.GetNestedObjectMapping () == InputMapping::Reflect ().mapping);
    _constructor.DependOn (Checkpoint::INPUT_DISPATCH_BEGIN);
    _constructor.MakeDependencyOf (Checkpoint::INPUT_DISPATCH_END);
    application->addInputListener (this);
}

InputCollector::~InputCollector () noexcept
{
    application->removeInputListener (this);
}

void InputCollector::Execute () noexcept
{
    auto singletonCursor = modifySingleton.Execute ();
    auto *mapping = static_cast<InputMapping *> (mappingField.GetValue (*singletonCursor));

    for (const InputSubscription &subscription : mapping->subscriptions)
    {
        auto listenerCursor = modifyListenersById.Execute (&subscription.listenerId);
        auto *listener = static_cast<InputListenerObject *> (*listenerCursor);
        listener->actions.Clear ();
    }

    for (const InputAction &action : mapping->currentFrameActions)
    {
        for (const InputSubscription &subscription : mapping->subscriptions)
        {
            if (subscription.group == action.group)
            {
                auto listenerCursor = modifyListenersById.Execute (&subscription.listenerId);
                auto *listener = static_cast<InputListenerObject *> (*listenerCursor);

                if (listener->actions.GetCount () < listener->actions.GetCapacity ())
                {
                    listener->actions.EmplaceBack (action);
                }
            }
        }
    }

    mapping->currentFrameActions.Clear ();
}

bool InputCollector::keyPressed (const OgreBites::KeyboardEvent &_event)
{
    auto singletonCursor = modifySingleton.Execute ();
    auto *mapping = static_cast<InputMapping *> (mappingField.GetValue (*singletonCursor));
    bool anySatisfied = false;

    for (KeyboardActionTrigger &trigger : mapping->keyboardTriggers)
    {
        bool satisfied = true;
        for (KeyRequirement &requirement : trigger.requirements)
        {
            if (requirement.key == _event.keysym.sym)
            {
                switch (requirement.currentKeyState)
                {
                case KeyState::JUST_PRESSED:
                    requirement.currentKeyState = KeyState::PRESSED;
                    break;
                case KeyState::PRESSED:
                    break;
                case KeyState::RELEASED:
                    requirement.currentKeyState = KeyState::JUST_PRESSED;
                    break;
                }
            }

            if (requirement.justPressed)
            {
                satisfied &= requirement.currentKeyState == KeyState::JUST_PRESSED;
            }
            else
            {
                satisfied &= requirement.currentKeyState == KeyState::JUST_PRESSED ||
                             requirement.currentKeyState == KeyState::PRESSED;
            }
        }

        if (satisfied)
        {
            anySatisfied = true;
            if (mapping->currentFrameActions.GetCount () < mapping->currentFrameActions.GetCapacity ())
            {
                mapping->currentFrameActions.EmplaceBack (trigger.action);
            }
        }
    }

    return anySatisfied;
}
bool InputCollector::keyReleased (const OgreBites::KeyboardEvent &_event)
{
    auto singletonCursor = modifySingleton.Execute ();
    auto *mapping = static_cast<InputMapping *> (mappingField.GetValue (*singletonCursor));

    for (KeyboardActionTrigger &trigger : mapping->keyboardTriggers)
    {
        for (KeyRequirement &requirement : trigger.requirements)
        {
            if (requirement.key == _event.keysym.sym)
            {
                requirement.currentKeyState = KeyState::RELEASED;
            }
        }
    }

    return false;
}

void AddFixedUpdateTask (OgreBites::ApplicationContext *_application,
                         Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    Emergence::Celerity::TaskConstructor constructor = _pipelineBuilder.AddTask ("InputCollector");
    constructor.SetExecutor (
        [state {std::make_shared<InputCollector> (_application, FixedInputMappingSingleton::Reflect ().mapping,
                                                  FixedInputMappingSingleton::Reflect ().inputMapping, constructor)}] ()
        {
            state->Execute ();
        });
}

void AddNormalUpdateTask (OgreBites::ApplicationContext *_application,
                          Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    Emergence::Celerity::TaskConstructor constructor = _pipelineBuilder.AddTask ("InputCollector");
    constructor.SetExecutor (
        [state {std::make_shared<InputCollector> (_application, NormalInputMappingSingleton::Reflect ().mapping,
                                                  NormalInputMappingSingleton::Reflect ().inputMapping,
                                                  constructor)}] ()
        {
            state->Execute ();
        });
}
} // namespace InputCollection
