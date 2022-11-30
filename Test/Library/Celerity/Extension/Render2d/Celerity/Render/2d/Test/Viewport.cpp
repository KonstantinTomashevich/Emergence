#include <Celerity/Render/2d/Test/Scenario.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Celerity::Test::Tasks;
using namespace Emergence::Celerity::Test;
using namespace Emergence::Celerity;
using namespace Emergence::Memory::Literals;

BEGIN_SUITE (Viewport)

TEST_CASE (ClearColor)
{
    ExecuteScenario ({
        TaskPoint {
            CreateViewport {"GameWorld"_us, 0u, 0u, 0u, WIDTH, HEIGHT, 0xAAAAAAFF},
            CreateTransform {0u, INVALID_UNIQUE_ID, {}},
            CreateCamera {0u, 2.0f, ~0u},
        },
        AssetWaitPoint {},
        ScreenShotPoint {"ViewportClearColor"_us},
    });
}

TEST_CASE (Several)
{
    ExecuteScenario ({
        TaskPoint {
            CreateViewport {"GameWorld1"_us, 0u, 0u, 0u, WIDTH / 2u, HEIGHT / 2u, 0x000000FF},
            CreateTransform {0u, INVALID_UNIQUE_ID, {{2.0f, 0.0f}}},
            CreateCamera {0u, 2.0f, ~0u},

            CreateViewport {"GameWorld2"_us, 1u, WIDTH / 2u, 0u, WIDTH / 2u, HEIGHT / 2u, 0x000000FF},
            CreateTransform {1u, INVALID_UNIQUE_ID, {{-2.0f, 0.0f}}},
            CreateCamera {1u, 2.0f, ~0u},

            CreateViewport {"GameWorld3"_us, 2u, 0u, HEIGHT / 2u, WIDTH / 2u, HEIGHT / 2u, 0x000000FF},
            CreateTransform {2u, INVALID_UNIQUE_ID, {{0.0f, 1.0f}}},
            CreateCamera {2u, 2.0f, ~0u},

            CreateViewport {"GameWorld4"_us, 3u, WIDTH / 2u, HEIGHT / 2u, WIDTH / 2u, HEIGHT / 2u, 0x000000FF},
            CreateTransform {3u, INVALID_UNIQUE_ID, {{0.0f, -1.0f}}},
            CreateCamera {3u, 2.0f, ~0u},

            CreateTransform {4u, INVALID_UNIQUE_ID, {}},
            CreateSprite {4u, 0u, "Crate"_us, {{0.0f, 0.0f}, {1.0f, 1.0f}}, {0.5f, 0.5f}, 1u, ~0u},
        },
        AssetWaitPoint {},
        ScreenShotPoint {"ViewportSeveral"_us},
    });
}

TEST_CASE (Overlap)
{
    ExecuteScenario ({
        TaskPoint {
            CreateViewport {"GameWorld"_us, 0u, 0u, 0u, WIDTH, HEIGHT, 0x000000FF, 0u},
            CreateTransform {0u, INVALID_UNIQUE_ID, {}},
            CreateCamera {0u, 2.0f, 1u},

            CreateViewport {"GameWorldOverlap"_us, 1u, 0u, 0u, WIDTH, HEIGHT, 0x00000000, 1u},
            CreateTransform {1u, INVALID_UNIQUE_ID, {}},
            CreateCamera {1u, 2.0f, 2u},

            CreateTransform {2u, INVALID_UNIQUE_ID, {}},
            CreateSprite {2u, 0u, "ChangedFlare"_us, {{0.0f, 0.0f}, {1.0f, 1.0f}}, {1.0f, 1.0f}, 1u, 1u},

            CreateTransform {3u, INVALID_UNIQUE_ID, {}},
            CreateSprite {3u, 1u, "Crate"_us, {{0.0f, 0.0f}, {1.0f, 1.0f}}, {0.5f, 0.5f}, 1u, 2u},
        },
        AssetWaitPoint {},
        ScreenShotPoint {"ViewportOverlap"_us},
    });
}

END_SUITE
