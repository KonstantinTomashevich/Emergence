#include <Celerity/Render/2d/Test/Scenario.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Celerity::Test::Tasks;
using namespace Emergence::Celerity::Test;
using namespace Emergence::Celerity;
using namespace Emergence::Memory::Literals;

BEGIN_SUITE (SpriteAnimation)

TEST_CASE (SpriteAnimationTick)
{
    ExecuteScenario ({
        TaskPoint {
            CreateViewport {"GameWorld"_us, 0u, 0u, 0u, WIDTH, HEIGHT, 0x000000FF},
            CreateTransform {0u, INVALID_UNIQUE_ID, {}},
            CreateCamera {0u, 2.0f, ~0u},

            CreateTransform {1u, INVALID_UNIQUE_ID, {}},
            CreateSprite {1u, 0u, "WarriorRun1"_us, {{0.0f, 0.0f}, {1.0f, 1.0f}}, {1.0f, 1.0f}, 0u, ~0u},
            CreateSpriteAnimation {1u, 0u, "WarriorRun1Animation"_us, 0u, true, false, false, false},
        },
        AssetWaitPoint {},
        ScreenShotPoint {"SpriteAnimationTick1"_us},
        FrameSkipPoint {5u},
        ScreenShotPoint {"SpriteAnimationTick2"_us},
        FrameSkipPoint {9u},
        ScreenShotPoint {"SpriteAnimationTick3"_us},
        FrameSkipPoint {13u},
        ScreenShotPoint {"SpriteAnimationTick4"_us},
    });
}

TEST_CASE (SpriteAnimationNoTick)
{
    ExecuteScenario ({
        TaskPoint {
            CreateViewport {"GameWorld"_us, 0u, 0u, 0u, WIDTH, HEIGHT, 0x000000FF},
            CreateTransform {0u, INVALID_UNIQUE_ID, {}},
            CreateCamera {0u, 2.0f, ~0u},

            CreateTransform {1u, INVALID_UNIQUE_ID, {}},
            CreateSprite {1u, 0u, "WarriorRun1"_us, {{0.0f, 0.0f}, {1.0f, 1.0f}}, {1.0f, 1.0f}, 0u, ~0u},
            CreateSpriteAnimation {1u, 0u, "WarriorRun1Animation"_us, 0u, false, false, false, false},
        },
        AssetWaitPoint {},
        ScreenShotPoint {"SpriteAnimationNoTick1"_us},
        FrameSkipPoint {25u},
        ScreenShotPoint {"SpriteAnimationNoTick2"_us},
    });
}

TEST_CASE (SpriteAnimationOverrideTime)
{
    ExecuteScenario ({
        TaskPoint {
            CreateViewport {"GameWorld"_us, 0u, 0u, 0u, WIDTH, HEIGHT, 0x000000FF},
            CreateTransform {0u, INVALID_UNIQUE_ID, {}},
            CreateCamera {0u, 2.0f, ~0u},

            CreateTransform {1u, INVALID_UNIQUE_ID, {}},
            CreateSprite {1u, 0u, "WarriorRun1"_us, {{0.0f, 0.0f}, {1.0f, 1.0f}}, {1.0f, 1.0f}, 0u, ~0u},
            CreateSpriteAnimation {1u, 0u, "WarriorRun1Animation"_us, 510000000u, false, false, false, false},
        },
        AssetWaitPoint {},
        ScreenShotPoint {"SpriteAnimationOverrideTime1"_us},
        TaskPoint {
            UpdateSpriteAnimation {0u, "WarriorRun1Animation"_us, 830000000u, false, false, false, false},
        },
        ScreenShotPoint {"SpriteAnimationOverrideTime2"_us},
    });
}

TEST_CASE (SpriteAnimationLoop)
{
    ExecuteScenario ({
        TaskPoint {
            CreateViewport {"GameWorld"_us, 0u, 0u, 0u, WIDTH, HEIGHT, 0x000000FF},
            CreateTransform {0u, INVALID_UNIQUE_ID, {}},
            CreateCamera {0u, 2.0f, ~0u},

            CreateTransform {1u, INVALID_UNIQUE_ID, {}},
            CreateSprite {1u, 0u, "WarriorRun1"_us, {{0.0f, 0.0f}, {1.0f, 1.0f}}, {1.0f, 1.0f}, 0u, ~0u},
            CreateSpriteAnimation {1u, 0u, "WarriorRun1Animation"_us, 880000000u, true, true, false, false},
        },
        AssetWaitPoint {},
        ScreenShotPoint {"SpriteAnimationLoop1"_us},
        FrameSkipPoint {5u},
        ScreenShotPoint {"SpriteAnimationLoop2"_us},
        FrameSkipPoint {9u},
        ScreenShotPoint {"SpriteAnimationLoop3"_us},
        FrameSkipPoint {13u},
        ScreenShotPoint {"SpriteAnimationLoop4"_us},
    });
}

TEST_CASE (SpriteAnimationNoLoop)
{
    ExecuteScenario ({
        TaskPoint {
            CreateViewport {"GameWorld"_us, 0u, 0u, 0u, WIDTH, HEIGHT, 0x000000FF},
            CreateTransform {0u, INVALID_UNIQUE_ID, {}},
            CreateCamera {0u, 2.0f, ~0u},

            CreateTransform {1u, INVALID_UNIQUE_ID, {}},
            CreateSprite {1u, 0u, "WarriorRun1"_us, {{0.0f, 0.0f}, {1.0f, 1.0f}}, {1.0f, 1.0f}, 0u, ~0u},
            CreateSpriteAnimation {1u, 0u, "WarriorRun1Animation"_us, 880000000u, true, false, false, false},
        },
        AssetWaitPoint {},
        ScreenShotPoint {"SpriteAnimationNoLoop1"_us},
        FrameSkipPoint {5u},
        ScreenShotPoint {"SpriteAnimationNoLoop2"_us},
        FrameSkipPoint {9u},
        ScreenShotPoint {"SpriteAnimationNoLoop3"_us},
        FrameSkipPoint {13u},
        ScreenShotPoint {"SpriteAnimationNoLoop4"_us},
    });
}

TEST_CASE (SpriteAnimationFlipU)
{
    ExecuteScenario ({
        TaskPoint {
            CreateViewport {"GameWorld"_us, 0u, 0u, 0u, WIDTH, HEIGHT, 0x000000FF},
            CreateTransform {0u, INVALID_UNIQUE_ID, {}},
            CreateCamera {0u, 2.0f, ~0u},

            CreateTransform {1u, INVALID_UNIQUE_ID, {}},
            CreateSprite {1u, 0u, "WarriorRun1"_us, {{0.0f, 0.0f}, {1.0f, 1.0f}}, {1.0f, 1.0f}, 0u, ~0u},
            CreateSpriteAnimation {1u, 0u, "WarriorRun1Animation"_us, 0u, false, false, true, false},
        },
        AssetWaitPoint {},
        ScreenShotPoint {"SpriteAnimationFlipU"_us},
    });
}

TEST_CASE (SpriteAnimationFlipV)
{
    ExecuteScenario ({
        TaskPoint {
            CreateViewport {"GameWorld"_us, 0u, 0u, 0u, WIDTH, HEIGHT, 0x000000FF},
            CreateTransform {0u, INVALID_UNIQUE_ID, {}},
            CreateCamera {0u, 2.0f, ~0u},

            CreateTransform {1u, INVALID_UNIQUE_ID, {}},
            CreateSprite {1u, 0u, "WarriorRun1"_us, {{0.0f, 0.0f}, {1.0f, 1.0f}}, {1.0f, 1.0f}, 0u, ~0u},
            CreateSpriteAnimation {1u, 0u, "WarriorRun1Animation"_us, 0u, false, false, false, true},
        },
        AssetWaitPoint {},
        ScreenShotPoint {"SpriteAnimationFlipV"_us},
    });
}

END_SUITE
