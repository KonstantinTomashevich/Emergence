#include <filesystem>
#include <fstream>

#include <Celerity/Physics/DynamicsMaterial.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Initialization/DynamicsMaterialLoading.hpp>

#include <Log/Log.hpp>

#include <Serialization/Yaml.hpp>

namespace DynamicsMaterialLoading
{
class DynamicsMaterialLoader final : public Emergence::Celerity::TaskExecutorBase<DynamicsMaterialLoader>
{
public:
    DynamicsMaterialLoader (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::InsertLongTermQuery insertDynamicsMaterial;
};

DynamicsMaterialLoader::DynamicsMaterialLoader (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : insertDynamicsMaterial (INSERT_LONG_TERM (Emergence::Celerity::DynamicsMaterial))
{
}

void DynamicsMaterialLoader::Execute () noexcept
{
    static const char *materialsPath = "../GameAssets/Physics/Materials/";
    auto cursor = insertDynamicsMaterial.Execute ();
    Emergence::Serialization::FieldNameLookupCache cache {Emergence::Celerity::DynamicsMaterial::Reflect ().mapping};

    for (const auto &entry : std::filesystem::directory_iterator (materialsPath))
    {
        if (entry.is_regular_file () && entry.path ().extension ().string () == ".yaml")
        {
            EMERGENCE_LOG (DEBUG, "DynamicsMaterialLoading: Loading \"", entry.path ().string ().c_str (), "\"...");
            std::ifstream input {entry};

            if (!input)
            {
                EMERGENCE_LOG (ERROR, "DynamicsMaterialLoading: Unable to open file \"",
                               entry.path ().string ().c_str (), "\"!");
            }

            auto *material = static_cast<Emergence::Celerity::DynamicsMaterial *> (++cursor);
            if (!Emergence::Serialization::Yaml::DeserializeObject (input, material, cache))
            {
                EMERGENCE_LOG (ERROR, "DynamicsMaterialLoading: Failed to load Dynamics material from \"",
                               entry.path ().string ().c_str (), "\"!");
            }
        }
    }
}

using namespace Emergence::Memory::Literals;

void AddToInitializationPipeline (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddTask ("DynamicsMaterialLoader"_us).SetExecutor<DynamicsMaterialLoader> ();
}
} // namespace DynamicsMaterialLoading
