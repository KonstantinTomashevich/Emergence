#include <filesystem>
#include <fstream>

#include <Celerity/Physics/DynamicsMaterial.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Loading/Model/DynamicsMaterialLoadingSingleton.hpp>
#include <Loading/Task/DynamicsMaterialLoading.hpp>

#include <Log/Log.hpp>

#include <Serialization/Yaml.hpp>

#include <SyntaxSugar/Time.hpp>

namespace DynamicsMaterialLoading
{
const Emergence::Memory::UniqueString Checkpoint::STEP_FINISHED {"DynamicsMaterialLoadingStepFinished"};

class DynamicsMaterialLoader final : public Emergence::Celerity::TaskExecutorBase<DynamicsMaterialLoader>
{
public:
    DynamicsMaterialLoader (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    static constexpr const char *MATERIALS_PATH = "../GameAssets/Physics/Materials/";

    Emergence::Celerity::ModifySingletonQuery modifyState;
    Emergence::Celerity::InsertLongTermQuery insertDynamicsMaterial;

    Emergence::Serialization::FieldNameLookupCache cache {Emergence::Celerity::DynamicsMaterial::Reflect ().mapping};
    std::filesystem::directory_iterator directoryIterator {MATERIALS_PATH};
};

DynamicsMaterialLoader::DynamicsMaterialLoader (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : modifyState (MODIFY_SINGLETON (DynamicsMaterialLoadingSingleton)),
      insertDynamicsMaterial (INSERT_LONG_TERM (Emergence::Celerity::DynamicsMaterial))
{
    _constructor.MakeDependencyOf (Checkpoint::STEP_FINISHED);
}

void DynamicsMaterialLoader::Execute () noexcept
{
    auto stateCursor = modifyState.Execute ();
    auto *state = static_cast<DynamicsMaterialLoadingSingleton *> (*stateCursor);

    if (state->finished)
    {
        return;
    }

    auto cursor = insertDynamicsMaterial.Execute ();
    const std::uint64_t startTime = Emergence::Time::NanosecondsSinceStartup ();
    constexpr std::uint64_t MAXIMUM_LOADING_TIME_PER_STEP = 16000000u; // 16ms

    while (directoryIterator != std::filesystem::end (directoryIterator))
    {
        if (Emergence::Time::NanosecondsSinceStartup () - startTime > MAXIMUM_LOADING_TIME_PER_STEP)
        {
            return;
        }

        const std::filesystem::directory_entry &entry = *directoryIterator;
        if (entry.is_regular_file () && entry.path ().extension ().string () == ".yaml")
        {
            EMERGENCE_LOG (DEBUG, "DynamicsMaterialLoading: Loading \"", entry.path ().string ().c_str (), "\"...");
            std::ifstream input {entry.path ()};

            if (!input)
            {
                EMERGENCE_LOG (ERROR, "DynamicsMaterialLoading: Unable to open file \"",
                               entry.path ().string ().c_str (), "\"!");
                continue;
            }

            auto *material = static_cast<Emergence::Celerity::DynamicsMaterial *> (++cursor);
            if (!Emergence::Serialization::Yaml::DeserializeObject (input, material, cache))
            {
                EMERGENCE_LOG (ERROR, "DynamicsMaterialLoading: Failed to load DynamicsMaterial from \"",
                               entry.path ().string ().c_str (), "\"!");
            }
        }

        ++directoryIterator;
    }

    state->finished = true;
}

using namespace Emergence::Memory::Literals;

void AddToLoadingPipeline (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddCheckpoint (Checkpoint::STEP_FINISHED);
    _pipelineBuilder.AddTask ("DynamicsMaterialLoader"_us).SetExecutor<DynamicsMaterialLoader> ();
}
} // namespace DynamicsMaterialLoading
