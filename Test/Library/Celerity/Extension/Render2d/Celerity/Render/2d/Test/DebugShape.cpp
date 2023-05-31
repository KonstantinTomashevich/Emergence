#include <Celerity/Render/2d/Test/Scenario.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Celerity::Test::Tasks;
using namespace Emergence::Celerity::Test;
using namespace Emergence::Celerity;
using namespace Emergence::Math;
using namespace Emergence::Memory::Literals;

BEGIN_SUITE (DebugShapeRender)

TEST_CASE (DebugBox)
{
    ExecuteScenario ({
        TaskPoint {
            CreateViewport {"GameWorld"_us, 0u, 0u, 0u, WIDTH, HEIGHT, 0x000000FF},
            CreateTransform {0u, INVALID_UNIQUE_ID, {}},
            CreateCamera {0u, 2.0f, ~0u},

            CreateTransform {1u, INVALID_UNIQUE_ID, {}},
            CreateDebugShape {1u, 0u, "MI_DebugRed"_us, Vector2f::ZERO, 0.0f, DebugShape2d {0.5f, 0.5f}},
        },
        AssetWaitPoint {},
        ScreenShotPoint {"DebugBox"_us},
    });
}

TEST_CASE (DebugCircle)
{
    ExecuteScenario ({
        TaskPoint {
            CreateViewport {"GameWorld"_us, 0u, 0u, 0u, WIDTH, HEIGHT, 0x000000FF},
            CreateTransform {0u, INVALID_UNIQUE_ID, {}},
            CreateCamera {0u, 2.0f, ~0u},

            CreateTransform {1u, INVALID_UNIQUE_ID, {}},
            CreateDebugShape {1u, 0u, "MI_DebugRed"_us, Vector2f::ZERO, 0.0f, DebugShape2d {0.5f}},
        },
        AssetWaitPoint {},
        ScreenShotPoint {"DebugCircle"_us},
    });
}

TEST_CASE (DebugLine)
{
    ExecuteScenario ({
        TaskPoint {
            CreateViewport {"GameWorld"_us, 0u, 0u, 0u, WIDTH, HEIGHT, 0x000000FF},
            CreateTransform {0u, INVALID_UNIQUE_ID, {}},
            CreateCamera {0u, 2.0f, ~0u},

            CreateTransform {1u, INVALID_UNIQUE_ID, {}},
            CreateDebugShape {1u, 0u, "MI_DebugRed"_us, Vector2f::ZERO, 0.0f, DebugShape2d {Vector2f {0.5f, 0.5f}}},
        },
        AssetWaitPoint {},
        ScreenShotPoint {"DebugLine"_us},
    });
}

TEST_CASE (DebugBoxTransform)
{
    ExecuteScenario ({
        TaskPoint {
            CreateViewport {"GameWorld"_us, 0u, 0u, 0u, WIDTH, HEIGHT, 0x000000FF},
            CreateTransform {0u, INVALID_UNIQUE_ID, {}},
            CreateCamera {0u, 2.0f, ~0u},

            CreateTransform {1u, INVALID_UNIQUE_ID, {{2.0f, 0.5f}, 0.0f, Vector2f::ONE}},
            CreateDebugShape {1u, 0u, "MI_DebugRed"_us, {-1.0f, -1.0f}, 1.0f, DebugShape2d {0.5f, 0.5f}},
        },
        AssetWaitPoint {},
        ScreenShotPoint {"DebugBoxTransform"_us},
    });
}

END_SUITE
