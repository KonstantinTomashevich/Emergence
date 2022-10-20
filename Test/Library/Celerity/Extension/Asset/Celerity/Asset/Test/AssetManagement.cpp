#include <Celerity/Asset/Test/Data.hpp>
#include <Celerity/Asset/Test/Scenario.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Celerity::Test::AssetStateUpdaterTasks;
using namespace Emergence::Celerity::Test::ConfiguratorTasks;
using namespace Emergence::Celerity::Test::VerifierTasks;
using namespace Emergence::Celerity::Test;
using namespace Emergence::Memory::Literals;

BEGIN_SUITE (AssetLifetime)

TEST_CASE (SingleAssetUser)
{
    ExecuteScenario ({
        {{
             SetAutomaticallyCleanUnusedAssets {true},
             AddSingleAssetUser {0u, "testInstance"_us},
         },
         {},
         {
             CheckAssetNotExists {"unknownInstance"_us},
             CheckAssetType {"testInstance"_us, FirstAssetType::Reflect ().mapping},
         }},
        {{
             ChangeSingleAssetUser {0u, "testInstanceChanged"_us},
         },
         {},
         {
             CheckAssetNotExists {"testInstance"_us},
             CheckAssetType {"testInstanceChanged"_us, FirstAssetType::Reflect ().mapping},
         }},
        {{
             RemoveSingleAssetUser {0u},
         },
         {},
         {
             CheckAssetNotExists {"testInstance"_us},
             CheckAssetNotExists {"testInstanceChanged"_us},
             CheckUnusedAssetCount {0u},
         }},
    });
}

TEST_CASE (MultipleAssetUser)
{
    ExecuteScenario ({
        {{
             SetAutomaticallyCleanUnusedAssets {true},
             AddMultipleAssetUser {0u, "testFirstInstance"_us, "testSecondInstance"_us, "testThirdInstance"_us},
         },
         {},
         {
             CheckAssetNotExists {"unknownInstance"_us},
             CheckAssetType {"testFirstInstance"_us, FirstAssetType::Reflect ().mapping},
             CheckAssetType {"testSecondInstance"_us, SecondAssetType::Reflect ().mapping},
             CheckAssetType {"testThirdInstance"_us, SecondAssetType::Reflect ().mapping},
         }},
        {{
             ChangeMultipleAssetUser {0u, "testFirstInstance"_us, "testSecondInstanceChanged"_us,
                                      "testThirdInstance"_us},
         },
         {},
         {
             CheckAssetType {"testFirstInstance"_us, FirstAssetType::Reflect ().mapping},
             CheckAssetNotExists {"testSecondInstance"_us},
             CheckAssetType {"testSecondInstanceChanged"_us, SecondAssetType::Reflect ().mapping},
             CheckAssetType {"testThirdInstance"_us, SecondAssetType::Reflect ().mapping},
         }},
        {{
             RemoveMultipleAssetUser {0u},
         },
         {},
         {
             CheckAssetNotExists {"testFirstInstance"_us},
             CheckAssetNotExists {"testSecondInstance"_us},
             CheckAssetNotExists {"testSecondInstanceChanged"_us},
             CheckAssetNotExists {"testThirdInstance"_us},
             CheckUnusedAssetCount {0u},
         }},
    });
}

END_SUITE

BEGIN_SUITE (UsageCounting)

TEST_CASE (AddSeveralAndRemoveOne)
{
    ExecuteScenario ({
        {{
             SetAutomaticallyCleanUnusedAssets {false},
             AddSingleAssetUser {0u, "testInstance"_us},
             AddSingleAssetUser {1u, "testInstance"_us},
         },
         {},
         {
             CheckAssetUsages {"testInstance"_us, 2u},
             CheckUnusedAssetCount {0u},
         }},
        {{
             RemoveSingleAssetUser {0u},
         },
         {},
         {
             CheckAssetUsages {"testInstance"_us, 1u},
             CheckUnusedAssetCount {0u},
         }},
    });
}

TEST_CASE (ChangeSingleAssetUser)
{
    ExecuteScenario ({
        {{
             SetAutomaticallyCleanUnusedAssets {false},
             AddSingleAssetUser {0u, "testInstance"_us},
         },
         {},
         {
             CheckAssetUsages {"testInstance"_us, 1u},
             CheckUnusedAssetCount {0u},
         }},
        {{
             ChangeSingleAssetUser {0u, "testInstanceChanged"_us},
         },
         {},
         {
             CheckAssetUsages {"testInstance"_us, 0u},
             CheckAssetUsages {"testInstanceChanged"_us, 1u},
             CheckUnusedAssetCount {1u},
         }},
    });
}

TEST_CASE (AddSeveralForDifferentAssets)
{
    ExecuteScenario ({
        {{
             SetAutomaticallyCleanUnusedAssets {false},
             AddSingleAssetUser {0u, "testFirstInstance"_us},
             AddSingleAssetUser {1u, "testSecondInstance"_us},
         },
         {},
         {
             CheckAssetUsages {"testFirstInstance"_us, 1u},
             CheckAssetUsages {"testSecondInstance"_us, 1u},
             CheckUnusedAssetCount {0u},
         }},
    });
}

TEST_CASE (AddWithEmptyAssetReference)
{
    ExecuteScenario ({
        {
            {
                SetAutomaticallyCleanUnusedAssets {false},
                AddSingleAssetUser {0u, {}},
            },
            {},
            {
                CheckAssetNotExists {{}},
                CheckUnusedAssetCount {0u},
            },
        },
    });
}

TEST_CASE (SeveralMultipleAssetUsers)
{
    ExecuteScenario ({
        {{
             SetAutomaticallyCleanUnusedAssets {false},
             AddMultipleAssetUser {0u, "testFirstInstance"_us, "testSecondInstance"_us, "testThirdInstance"_us},
             AddMultipleAssetUser {1u, "testFirstInstance"_us, "testSecondInstanceOther"_us, "testThirdInstance"_us},
         },
         {},
         {
             CheckAssetUsages {"testFirstInstance"_us, 2u},
             CheckAssetUsages {"testSecondInstance"_us, 1u},
             CheckAssetUsages {"testSecondInstanceOther"_us, 1u},
             CheckAssetUsages {"testThirdInstance"_us, 2u},
             CheckUnusedAssetCount {0u},
         }},
        {{
             ChangeMultipleAssetUser {0u, "testFirstInstance"_us, "testSecondInstanceOther"_us, "testThirdInstance"_us},
         },
         {},
         {
             CheckAssetUsages {"testFirstInstance"_us, 2u},
             CheckAssetUsages {"testSecondInstance"_us, 0u},
             CheckAssetUsages {"testSecondInstanceOther"_us, 2u},
             CheckAssetUsages {"testThirdInstance"_us, 2u},
             CheckUnusedAssetCount {1u},
         }},
        {{
             RemoveMultipleAssetUser {1u},
         },
         {},
         {
             CheckAssetUsages {"testFirstInstance"_us, 1u},
             CheckAssetUsages {"testSecondInstance"_us, 0u},
             CheckAssetUsages {"testSecondInstanceOther"_us, 1u},
             CheckAssetUsages {"testThirdInstance"_us, 1u},
             CheckUnusedAssetCount {1u},
         }},
    });
}

END_SUITE

BEGIN_SUITE (StatusUpdate)

TEST_CASE (LoadMissing)
{
    ExecuteScenario ({
        {
            {
                AddSingleAssetUser {0u, "testInstance"_us},
            },
            {},
            {
                CheckAssetsLeftToLoad {1u},
            },
        },
        {
            {},
            {
                UpdateFirstAssetType {"testInstance"_us, Emergence::Celerity::AssetState::MISSING},
            },
            {
                CheckAssetsLeftToLoad {0u},
                CheckAssetState {"testInstance"_us, Emergence::Celerity::AssetState::MISSING},
            },
        },
    });
}

TEST_CASE (LoadCorrupted)
{
    ExecuteScenario ({
        {
            {
                AddSingleAssetUser {0u, "testInstance"_us},
            },
            {},
            {
                CheckAssetsLeftToLoad {1u},
            },
        },
        {
            {},
            {
                UpdateFirstAssetType {"testInstance"_us, Emergence::Celerity::AssetState::CORRUPTED},
            },
            {
                CheckAssetsLeftToLoad {0u},
                CheckAssetState {"testInstance"_us, Emergence::Celerity::AssetState::CORRUPTED},
            },
        },
    });
}

TEST_CASE (LoadReady)
{
    ExecuteScenario ({
        {
            {
                AddSingleAssetUser {0u, "testInstance"_us},
            },
            {},
            {
                CheckAssetsLeftToLoad {1u},
            },
        },
        {
            {},
            {
                UpdateFirstAssetType {"testInstance"_us, Emergence::Celerity::AssetState::READY},
            },
            {
                CheckAssetsLeftToLoad {0u},
                CheckAssetState {"testInstance"_us, Emergence::Celerity::AssetState::READY},
            },
        },
    });
}

TEST_CASE (LoadInstant)
{
    ExecuteScenario ({
        {
            {
                AddSingleAssetUser {0u, "testInstance"_us},
            },
            {
                UpdateFirstAssetType {"testInstance"_us, Emergence::Celerity::AssetState::READY},
            },
            {
                CheckAssetsLeftToLoad {0u},
                CheckAssetState {"testInstance"_us, Emergence::Celerity::AssetState::READY},
            },
        },
    });
}

TEST_CASE (LoadRemoval)
{
    ExecuteScenario ({
        {
            {
                AddSingleAssetUser {0u, "testInstance"_us},
            },
            {},
            {
                CheckAssetsLeftToLoad {1u},
            },
        },
        {
            {RemoveSingleAssetUser {0u}},
            {},
            {
                CheckAssetsLeftToLoad {0u},
            },
        },
    });
}

TEST_CASE (LoadMultipleAssetUser)
{
    ExecuteScenario ({
        {{
             AddMultipleAssetUser {0u, "testFirstInstance"_us, "testSecondInstance"_us, "testThirdInstance"_us},
         },
         {},
         {
             CheckAssetsLeftToLoad {3u},
         }},
        {
            {},
            {
                UpdateFirstAssetType {"testFirstInstance"_us, Emergence::Celerity::AssetState::READY},
                UpdateSecondAssetType {"testThirdInstance"_us, Emergence::Celerity::AssetState::READY},
            },
            {
                CheckAssetState {"testFirstInstance"_us, Emergence::Celerity::AssetState::READY},
                CheckAssetState {"testSecondInstance"_us, Emergence::Celerity::AssetState::LOADING},
                CheckAssetState {"testThirdInstance"_us, Emergence::Celerity::AssetState::READY},
                CheckAssetsLeftToLoad {1u},
            },
        },
    });
}

END_SUITE
