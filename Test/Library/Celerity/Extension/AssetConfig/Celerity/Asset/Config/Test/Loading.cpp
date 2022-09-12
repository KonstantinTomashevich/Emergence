#include <Celerity/Asset/Config/Test/Scenario.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Celerity::Test;
using namespace Emergence::Celerity::Test::Tasks;
using namespace Emergence::Memory::Literals;

static const Emergence::Container::String UNIT_CONFIG_DIR = "Config/Unit";

static const Emergence::Container::Vector<UnitConfig> UNIT_CONFIG {
    {"Footman"_us, 100.0f, 20.0f, 30.0f, 2.0f},
    {"Archer"_us, 100.0f, 0.0f, 70.0f, 5.0f},
    {"Mage"_us, 75.0f, 0.0f, 100.0f, 6.0f},
};

static const Emergence::Container::String BUILDING_CONFIG_DIR = "Config/Building";

static const Emergence::Container::Vector<BuildingConfig> BUILDING_CONFIG {
    {"Tower"_us, 2000.0f, 150.0f, 90.0f},
    {"Farm"_us, 750.0f, 100.0f, 45.0f},
    {"Barracks"_us, 1250.0f, 125.0f, 60.0f},
};

static const Emergence::Container::Vector<BuildingConfig> CHANGED_BUILDING_CONFIG {
    {"Tower"_us, 2100.0f, 160.0f, 100.0f},
    {"Farm"_us, 700.0f, 100.0f, 40.0f},
    {"Barracks"_us, 100.0f, 125.0f, 60.0f},
};

BEGIN_SUITE (AssetConfigLoading)

TEST_CASE (LoadAll)
{
    ExecuteScenario ({
        ResetEnvironment {UNIT_CONFIG_DIR, UNIT_CONFIG, BUILDING_CONFIG_DIR, BUILDING_CONFIG, false},
        LoadConfig {UnitConfig::Reflect ().mapping},
        LoadConfig {BuildingConfig::Reflect ().mapping},
        CheckUnitConfig {UNIT_CONFIG},
        CheckBuildingConfig {BUILDING_CONFIG},
    });
}

TEST_CASE (LoadAllBinary)
{
    ExecuteScenario ({
        ResetEnvironment {UNIT_CONFIG_DIR, UNIT_CONFIG, BUILDING_CONFIG_DIR, BUILDING_CONFIG, true},
        LoadConfig {UnitConfig::Reflect ().mapping},
        LoadConfig {BuildingConfig::Reflect ().mapping},
        CheckUnitConfig {UNIT_CONFIG},
        CheckBuildingConfig {BUILDING_CONFIG},
    });
}

TEST_CASE (LoadOnlyUnitConfig)
{
    ExecuteScenario ({
        ResetEnvironment {UNIT_CONFIG_DIR, UNIT_CONFIG, BUILDING_CONFIG_DIR, BUILDING_CONFIG, false},
        LoadConfig {UnitConfig::Reflect ().mapping},
        CheckUnitConfig {UNIT_CONFIG},
        CheckBuildingConfig {{}},
    });
}

TEST_CASE (LoadOnlyBuildingConfig)
{
    ExecuteScenario ({
        ResetEnvironment {UNIT_CONFIG_DIR, UNIT_CONFIG, BUILDING_CONFIG_DIR, BUILDING_CONFIG, false},
        LoadConfig {BuildingConfig::Reflect ().mapping},
        CheckUnitConfig {{}},
        CheckBuildingConfig {BUILDING_CONFIG},
    });
}

TEST_CASE (ReloadNotForced)
{
    ExecuteScenario ({
        ResetEnvironment {UNIT_CONFIG_DIR, UNIT_CONFIG, BUILDING_CONFIG_DIR, BUILDING_CONFIG, false},
        LoadConfig {BuildingConfig::Reflect ().mapping},
        CheckBuildingConfig {BUILDING_CONFIG},
        ResetEnvironment {UNIT_CONFIG_DIR, UNIT_CONFIG, BUILDING_CONFIG_DIR, CHANGED_BUILDING_CONFIG, false},
        LoadConfig {BuildingConfig::Reflect ().mapping},
        CheckBuildingConfig {BUILDING_CONFIG},
    });
}

TEST_CASE (ReloadForced)
{
    ExecuteScenario ({
        ResetEnvironment {UNIT_CONFIG_DIR, UNIT_CONFIG, BUILDING_CONFIG_DIR, BUILDING_CONFIG, false},
        LoadConfig {BuildingConfig::Reflect ().mapping},
        CheckBuildingConfig {BUILDING_CONFIG},
        ResetEnvironment {UNIT_CONFIG_DIR, UNIT_CONFIG, BUILDING_CONFIG_DIR, CHANGED_BUILDING_CONFIG, false},
        LoadConfig {BuildingConfig::Reflect ().mapping, true},
        CheckBuildingConfig {CHANGED_BUILDING_CONFIG},
    });
}

END_SUITE
