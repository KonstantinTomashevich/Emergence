#include <Celerity/Render/2d/Test/Scenario.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Celerity::Test::Tasks;
using namespace Emergence::Celerity::Test;
using namespace Emergence::Celerity;
using namespace Emergence::Memory::Literals;

BEGIN_SUITE (PostProcess)

TEST_CASE (RedOnly)
{
    ExecuteScenario ({
        TaskPoint {
            CreateScreenLikeFrameBuffer {"GameWorldBuffer"_us},

            CreateViewport {"GameWorld"_us, "GameWorldBuffer"_us, 0u, 0u, WIDTH, HEIGHT, 0x000000FF, 0u},
            CreateWorldRenderPass {"GameWorld"_us, 0u},
            CreateTransform {0u, INVALID_UNIQUE_ID, {{2.0f, 1.0f}}},
            CreateCamera {0u, 2.0f, ~0u},

            CreateTransform {1u, INVALID_UNIQUE_ID, {}},
            CreateSprite {1u, 0u, "MI_Crate"_us, {{0.0f, 0.0f}, {1.0f, 1.0f}}, {0.5f, 0.5f}, 0u, ~0u},

            CreateViewport {"PostProcess"_us, ""_us, 0u, 0u, WIDTH, HEIGHT, 0x000000FF, 1u},
            CreatePostProcessRenderPass {"PostProcess"_us, "MI_PostProcess_RedOnly"_us},
        },
        AssetWaitPoint {},
        ScreenShotPoint {"PostProcessRedOnly"_us},
    });
}

END_SUITE
