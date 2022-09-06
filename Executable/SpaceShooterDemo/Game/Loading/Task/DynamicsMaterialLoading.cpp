#include <filesystem>
#include <fstream>

#include <Celerity/Physics/DynamicsMaterial.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Loading/Model/Messages.hpp>
#include <Loading/Task/DynamicsMaterialLoading.hpp>

#include <Log/Log.hpp>

#include <Serialization/Yaml.hpp>

#include <SyntaxSugar/Time.hpp>

namespace DynamicsMaterialLoading
{
// TODO: Create shared generic object loader task?

const Emergence::Memory::UniqueString Checkpoint::FINISHED {"DynamicsMaterialLoadingFinished"};

class DynamicsMaterialLoader final : public Emergence::Celerity::TaskExecutorBase<DynamicsMaterialLoader>
{
public:
    DynamicsMaterialLoader (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::ModifySequenceQuery modifyRequests;
    Emergence::Celerity::ModifySequenceQuery modifyResponses;

    Emergence::Celerity::InsertLongTermQuery insertDynamicsMaterial;
    Emergence::Celerity::InsertShortTermQuery insertResponse;

    Emergence::Container::String currentFolder;
    Emergence::Serialization::FieldNameLookupCache cache {Emergence::Celerity::DynamicsMaterial::Reflect ().mapping};
    std::filesystem::directory_iterator directoryIterator;
};

DynamicsMaterialLoader::DynamicsMaterialLoader (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : modifyRequests (MODIFY_SEQUENCE (DynamicsMaterialLoadingRequest)),
     modifyResponses (MODIFY_SEQUENCE (DynamicsMaterialLoadingFinishedResponse)),
      insertDynamicsMaterial (INSERT_LONG_TERM (Emergence::Celerity::DynamicsMaterial)),
      insertResponse (INSERT_SHORT_TERM (DynamicsMaterialLoadingFinishedResponse))
{
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
}

void DynamicsMaterialLoader::Execute () noexcept
{
    // Clear old responses.
    for (auto responseCursor = modifyResponses.Execute(); *responseCursor; ~responseCursor)
    {
    }

    auto cursor = insertDynamicsMaterial.Execute ();
    const std::uint64_t startTime = Emergence::Time::NanosecondsSinceStartup ();
    constexpr std::uint64_t MAXIMUM_LOADING_TIME_PER_STEP = 16000000u; // 16ms

    while (true)
    {
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

        if (!currentFolder.empty ())
        {
            auto insertionCursor = insertResponse.Execute ();
            auto *response = static_cast<DynamicsMaterialLoadingFinishedResponse *> (++insertionCursor);
            response->folder = std::move (currentFolder);
        }

        auto requestCursor = modifyRequests.Execute ();
        if (auto *request = static_cast<DynamicsMaterialLoadingRequest *> (*requestCursor))
        {
            currentFolder = std::move (request->folder);
            directoryIterator = std::filesystem::directory_iterator {currentFolder};
            ~requestCursor;
        }
        else
        {
            // No more requests, everything is loaded.
            return;
        }
    }
}

using namespace Emergence::Memory::Literals;

void AddToLoadingPipeline (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);
    _pipelineBuilder.AddTask ("DynamicsMaterialLoader"_us).SetExecutor<DynamicsMaterialLoader> ();
}
} // namespace DynamicsMaterialLoading
