#pragma once

#include <variant>

#include <Celerity/Asset/Config/Test/Types.hpp>

#include <Container/String.hpp>
#include <Container/Vector.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity::Test
{
namespace Tasks
{
struct ResetEnvironment final
{
    Container::String unitConfigFolder;
    Container::Vector<UnitConfig> unitConfigs;

    Container::String buildingConfigFolder;
    Container::Vector<BuildingConfig> buildingConfigs;

    bool useBinaryFormat = false;
};

struct LoadConfig final
{
    StandardLayout::Mapping type;
    bool forceReload = false;
};

struct CheckUnitConfig final
{
    Container::Vector<UnitConfig> unitConfigs;
};

struct CheckBuildingConfig final
{
    Container::Vector<BuildingConfig> buildingConfigs;
};
} // namespace Tasks

using Task =
    std::variant<Tasks::ResetEnvironment, Tasks::LoadConfig, Tasks::CheckUnitConfig, Tasks::CheckBuildingConfig>;

void ExecuteScenario (const Container::Vector<Task> &_tasks) noexcept;
} // namespace Emergence::Celerity::Test
