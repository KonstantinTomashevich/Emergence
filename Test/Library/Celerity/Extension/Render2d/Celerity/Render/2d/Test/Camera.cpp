#include <Celerity/Render/2d/Test/Scenario.hpp>

#include <Math/Constants.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Celerity::Test::Tasks;
using namespace Emergence::Celerity::Test;
using namespace Emergence::Celerity;
using namespace Emergence::Memory::Literals;

BEGIN_SUITE (Camera)

TEST_CASE (TranslateCamera)
{
    ExecuteScenario ({
        TaskPoint {
            CreateViewport {"GameWorld"_us, ""_us, 0u, 0u, WIDTH, HEIGHT, 0x000000FF},
            CreateWorldRenderPass {"GameWorld"_us, 0u},
            CreateTransform {0u, INVALID_UNIQUE_ID, {{2.0f, 1.0f}}},
            CreateCamera {0u, 2.0f, ~0u},

            CreateTransform {1u, INVALID_UNIQUE_ID, {}},
            CreateSprite {1u, 0u, "MI_Crate"_us, {{0.0f, 0.0f}, {1.0f, 1.0f}}, {0.5f, 0.5f}, 0u, ~0u},
        },
        AssetWaitPoint {},
        ScreenShotPoint {"TranslateCamera"_us},
    });
}

TEST_CASE (RotateCamera)
{
    ExecuteScenario ({
        TaskPoint {
            CreateViewport {"GameWorld"_us, ""_us, 0u, 0u, WIDTH, HEIGHT, 0x000000FF},
            CreateWorldRenderPass {"GameWorld"_us, 0u},
            CreateTransform {0u, INVALID_UNIQUE_ID, {{0.0f, 0.0f}, -Emergence::Math::PI / 6.0f}},
            CreateCamera {0u, 2.0f, ~0u},

            CreateTransform {1u, INVALID_UNIQUE_ID, {{2.0f, 0.0f}}},
            CreateSprite {1u, 0u, "MI_Crate"_us, {{0.0f, 0.0f}, {1.0f, 1.0f}}, {0.5f, 0.5f}, 0u, ~0u},
        },
        AssetWaitPoint {},
        ScreenShotPoint {"RotateCamera"_us},
    });
}

TEST_CASE (ScaleCamera)
{
    ExecuteScenario ({
        TaskPoint {
            CreateViewport {"GameWorld"_us, ""_us, 0u, 0u, WIDTH, HEIGHT, 0x000000FF},
            CreateWorldRenderPass {"GameWorld"_us, 0u},
            CreateTransform {0u, INVALID_UNIQUE_ID, {{0.0f, 0.0f}, 0.0f, {2.0f, 1.5f}}},
            CreateCamera {0u, 2.0f, ~0u},

            CreateTransform {1u, INVALID_UNIQUE_ID, {{1.0f, 0.0f}}},
            CreateSprite {1u, 0u, "MI_Crate"_us, {{0.0f, 0.0f}, {1.0f, 1.0f}}, {0.5f, 0.5f}, 0u, ~0u},
        },
        AssetWaitPoint {},
        ScreenShotPoint {"ScaleCamera"_us},
    });
}

TEST_CASE (SwitchCamera)
{
    ExecuteScenario ({
        TaskPoint {
            CreateViewport {"GameWorld"_us, ""_us, 0u, 0u, WIDTH, HEIGHT, 0x000000FF},
            CreateWorldRenderPass {"GameWorld"_us, 0u},
            CreateTransform {0u, INVALID_UNIQUE_ID, {{2.0f, 0.0f}}},
            CreateCamera {0u, 2.0f, ~0u},

            CreateTransform {1u, INVALID_UNIQUE_ID, {{-5.0f, 0.0f}}},
            CreateCamera {1u, 4.0f, ~0u},

            CreateTransform {2u, INVALID_UNIQUE_ID, {}},
            CreateSprite {2u, 0u, "MI_Crate"_us, {{0.0f, 0.0f}, {1.0f, 1.0f}}, {0.5f, 0.5f}, 0u, ~0u},
        },
        AssetWaitPoint {},
        ScreenShotPoint {"SwitchCameraFirst"_us},
        TaskPoint {
            UpdateWorldRenderPass {"GameWorld"_us, 1u},
        },
        ScreenShotPoint {"SwitchCameraSecond"_us},
    });
}

END_SUITE
