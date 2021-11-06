#include <cassert>

#include <Input/InputConfigurationSingleton.hpp>
#include <Input/InputListenerObject.hpp>
#include <Input/InputRoutingTask.hpp>

#include <Log/Log.hpp>

#include <Shared/Checkpoint.hpp>

class InputRouter final
{
public:
    InputRouter (InputAccumulator *_source, Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute ();

private:
    InputAccumulator *source;
    Emergence::Warehouse::FetchSingletonQuery fetchInputConfigurationSingleton;
    Emergence::Warehouse::ModifyValueQuery modifyInputListenerObjectById;
};

InputRouter::InputRouter (InputAccumulator *_source, Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : source (_source),
      fetchInputConfigurationSingleton (_constructor.FetchSingleton (InputConfigurationSingleton::Reflect ().mapping)),
      modifyInputListenerObjectById (_constructor.ModifyValue (InputListenerObject::Reflect ().mapping,
                                                               {InputListenerObject::Reflect ().objectId}))
{
    assert (source);
    _constructor.MakeDependencyOf (Checkpoint::INPUT_ROUTING_FINISHED);
}

void InputRouter::Execute ()
{
    auto configurationSingletonCursor = fetchInputConfigurationSingleton.Execute ();
    const auto *inputConfigurationSingleton =
        static_cast<const InputConfigurationSingleton *> (*configurationSingletonCursor);

    if (!inputConfigurationSingleton->readyForRouting)
    {
        return;
    }

    uint64_t targetListenerId = inputConfigurationSingleton->listenerObjectId;
    auto listenerCursor = modifyInputListenerObjectById.Execute (&targetListenerId);
    auto *inputListenerObject = static_cast<InputListenerObject *> (*listenerCursor);

    if (!inputListenerObject)
    {
        Emergence::Log::GlobalLogger::Log (
            Emergence::Log::Level::ERROR,
            "Unable to find InputListenerObject with id " + std::to_string (targetListenerId) + " for input routing!");
        return;
    }

#ifndef NDEBUG
    ++listenerCursor;
    if (*listenerCursor)
    {
        Emergence::Log::GlobalLogger::Log (
            Emergence::Log::Level::ERROR,
            "Found more than one InputListenerObject with id " + std::to_string (targetListenerId) + "!");
    }
#endif

    auto iterator = inputListenerObject->actions.begin ();
    // TODO: Get correct time from time singleton.
    while (iterator != inputListenerObject->actions.end () && source->PopNextAction (*iterator, 0u))
    {
        ++iterator;
    }
}

void AddInputRoutingTask (InputAccumulator *_source, Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    Emergence::Celerity::TaskConstructor constructor = _pipelineBuilder.AddTask ("InputRouting");
    constructor.SetExecutor (
        [state {std::make_shared<InputRouter> (_source, constructor)}] ()
        {
            state->Execute ();
        });
}
