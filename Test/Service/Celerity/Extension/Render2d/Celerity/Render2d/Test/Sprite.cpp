#include <Celerity/Render2d/Test/Scenario.hpp>
#include <Celerity/Render2d/Test/Sprite.hpp>

#include <Math/Constants.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Celerity::Test
{
bool SpriteTestIncludeMarker () noexcept
{
    return true;
}
} // namespace Emergence::Celerity::Test

using namespace Emergence::Celerity::Test::Tasks;
using namespace Emergence::Celerity::Test;
using namespace Emergence::Celerity;
using namespace Emergence::Memory::Literals;

BEGIN_SUITE (SpriteRender)

TEST_CASE (CrateSprite)
{
    ExecuteScenario ({
        TaskPoint {
            CreateTransform {0u, INVALID_UNIQUE_ID, {}},
            CreateCamera {0u, 2.0f, ~0u},
            SetCurrentCamera {0u},

            CreateTransform {1u, INVALID_UNIQUE_ID, {}},
            CreateSprite {1u, 0u, "Crate"_us, {{0.0f, 0.0f}, {1.0f, 1.0f}}, {1.0f, 1.0f}, 0u, ~0u},
        },
        AssetWaitPoint {},
        ScreenShotPoint {"CrateSprite"_us},
    });
}

TEST_CASE (UV)
{
    ExecuteScenario ({
        TaskPoint {
            CreateTransform {0u, INVALID_UNIQUE_ID, {}},
            CreateCamera {0u, 2.0f, ~0u},
            SetCurrentCamera {0u},

            CreateTransform {1u, INVALID_UNIQUE_ID, {}},
            CreateSprite {1u, 0u, "Crate"_us, {{0.0f, 0.0f}, {0.5f, 0.5f}}, {1.0f, 1.0f}, 0u, ~0u},
        },
        AssetWaitPoint {},
        ScreenShotPoint {"UV"_us},
    });
}

TEST_CASE (TransparentSprite)
{
    ExecuteScenario ({
        TaskPoint {
            CreateTransform {0u, INVALID_UNIQUE_ID, {}},
            CreateCamera {0u, 2.0f, ~0u},
            SetCurrentCamera {0u},

            CreateTransform {1u, INVALID_UNIQUE_ID, {}},
            CreateSprite {1u, 0u, "TransparentSprite"_us, {{0.0f, 0.0f}, {1.0f, 1.0f}}, {1.0f, 1.0f}, 0u, ~0u},
        },
        AssetWaitPoint {},
        ScreenShotPoint {"TransparentSprite"_us},
    });
}

TEST_CASE (MultipleSpritesOnOneObject)
{
    ExecuteScenario ({
        TaskPoint {
            CreateTransform {0u, INVALID_UNIQUE_ID, {}},
            CreateCamera {0u, 2.0f, ~0u},
            SetCurrentCamera {0u},

            CreateTransform {1u, INVALID_UNIQUE_ID, {}},
            CreateSprite {1u, 0u, "TransparentSprite"_us, {{0.0f, 0.0f}, {1.0f, 1.0f}}, {1.0f, 1.0f}, 0u, ~0u},
            CreateSprite {1u, 1u, "TransparentSprite"_us, {{0.0f, 0.0f}, {1.0f, 1.0f}}, {0.5f, 0.5f}, 0u, ~0u},
        },
        AssetWaitPoint {},
        ScreenShotPoint {"MultipleSpritesOnOneObject"_us},
    });
}

TEST_CASE (CustomShader)
{
    ExecuteScenario ({
        TaskPoint {
            CreateTransform {0u, INVALID_UNIQUE_ID, {}},
            CreateCamera {0u, 2.0f, ~0u},
            SetCurrentCamera {0u},

            CreateTransform {1u, INVALID_UNIQUE_ID, {}},
            CreateSprite {1u, 0u, "BaseFlare"_us, {{0.0f, 0.0f}, {1.0f, 1.0f}}, {1.0f, 1.0f}, 0u, ~0u},
        },
        AssetWaitPoint {},
        ScreenShotPoint {"CustomShader"_us},
    });
}

TEST_CASE (MaterialInstanceInheritance)
{
    ExecuteScenario ({
        TaskPoint {
            CreateTransform {0u, INVALID_UNIQUE_ID, {}},
            CreateCamera {0u, 2.0f, ~0u},
            SetCurrentCamera {0u},

            CreateTransform {1u, INVALID_UNIQUE_ID, {}},
            CreateSprite {1u, 0u, "ChangedFlare"_us, {{0.0f, 0.0f}, {1.0f, 1.0f}}, {1.0f, 1.0f}, 0u, ~0u},
        },
        AssetWaitPoint {},
        ScreenShotPoint {"MaterialInstanceInheritance"_us},
    });
}

TEST_CASE (Layers)
{
    ExecuteScenario ({
        TaskPoint {
            CreateTransform {0u, INVALID_UNIQUE_ID, {}},
            CreateCamera {0u, 2.0f, ~0u},
            SetCurrentCamera {0u},

            CreateTransform {1u, INVALID_UNIQUE_ID, {}},
            CreateSprite {1u, 0u, "TransparentSprite"_us, {{0.0f, 0.0f}, {1.0f, 1.0f}}, {1.0f, 1.0f}, 0u, ~0u},
            CreateSprite {1u, 1u, "TransparentSprite"_us, {{0.0f, 0.0f}, {1.0f, 1.0f}}, {0.5f, 0.5f}, 2u, ~0u},
            CreateSprite {1u, 2u, "Crate"_us, {{0.0f, 0.0f}, {1.0f, 1.0f}}, {0.5f, 0.5f}, 1u, ~0u},
        },
        AssetWaitPoint {},
        ScreenShotPoint {"Layers"_us},
    });
}

TEST_CASE (UpdateSprite)
{
    ExecuteScenario ({
        TaskPoint {
            CreateTransform {0u, INVALID_UNIQUE_ID, {}},
            CreateCamera {0u, 2.0f, ~0u},
            SetCurrentCamera {0u},

            CreateTransform {1u, INVALID_UNIQUE_ID, {}},
            CreateSprite {1u, 0u, "Crate"_us, {{0.0f, 0.0f}, {1.0f, 1.0f}}, {0.5f, 0.5f}, 1u, ~0u},
        },
        AssetWaitPoint {},
        TaskPoint {
            UpdateSprite {0u, "BaseFlare"_us, {{0.0f, 0.0f}, {1.0f, 1.0f}}, {1.0f, 1.0f}, 1u, ~0u},
        },
        AssetWaitPoint {},
        ScreenShotPoint {"UpdateSprite"_us},
    });
}

TEST_CASE (SpriteVisibilityMask)
{
    ExecuteScenario ({
        TaskPoint {
            CreateTransform {0u, INVALID_UNIQUE_ID, {}},
            CreateCamera {0u, 2.0f, 1u},
            SetCurrentCamera {0u},

            CreateTransform {1u, INVALID_UNIQUE_ID, {}},
            CreateSprite {1u, 0u, "TransparentSprite"_us, {{0.0f, 0.0f}, {1.0f, 1.0f}}, {1.0f, 1.0f}, 0u, 1u},
            CreateSprite {1u, 1u, "TransparentSprite"_us, {{0.0f, 0.0f}, {1.0f, 1.0f}}, {0.5f, 0.5f}, 0u, 2u},
        },
        AssetWaitPoint {},
        ScreenShotPoint {"SpriteVisibilityMask"_us},
    });
}

END_SUITE

BEGIN_SUITE (SpriteTransform)

TEST_CASE (OutOfCamera)
{
    ExecuteScenario ({
        TaskPoint {
            CreateTransform {0u, INVALID_UNIQUE_ID, {}},
            CreateCamera {0u, 2.0f, ~0u},
            SetCurrentCamera {0u},

            CreateTransform {1u, INVALID_UNIQUE_ID, {{10.0f, 0.0f}}},
            CreateSprite {1u, 1u, "Crate"_us, {{0.0f, 0.0f}, {1.0f, 1.0f}}, {0.5f, 0.5f}, 1u, ~0u},
        },
        AssetWaitPoint {},
        ScreenShotPoint {"OutOfCamera"_us},
    });
}

TEST_CASE (Translation)
{
    ExecuteScenario ({
        TaskPoint {
            CreateTransform {0u, INVALID_UNIQUE_ID, {}},
            CreateCamera {0u, 2.0f, ~0u},
            SetCurrentCamera {0u},

            CreateTransform {1u, INVALID_UNIQUE_ID, {{1.0f, 1.0f}}},
            CreateSprite {1u, 1u, "Crate"_us, {{0.0f, 0.0f}, {1.0f, 1.0f}}, {0.5f, 0.5f}, 1u, ~0u},
        },
        AssetWaitPoint {},
        ScreenShotPoint {"Translation"_us},
    });
}

TEST_CASE (TranslationRotation)
{
    ExecuteScenario ({
        TaskPoint {
            CreateTransform {0u, INVALID_UNIQUE_ID, {}},
            CreateCamera {0u, 2.0f, ~0u},
            SetCurrentCamera {0u},

            CreateTransform {1u, INVALID_UNIQUE_ID, {{1.0f, 1.0f}, Emergence::Math::PI / 6.0f}},
            CreateSprite {1u, 1u, "Crate"_us, {{0.0f, 0.0f}, {1.0f, 1.0f}}, {0.5f, 0.5f}, 1u, ~0u},
        },
        AssetWaitPoint {},
        ScreenShotPoint {"TranslationRotation"_us},
    });
}

TEST_CASE (TranslationRotationScale)
{
    ExecuteScenario ({
        TaskPoint {
            CreateTransform {0u, INVALID_UNIQUE_ID, {}},
            CreateCamera {0u, 2.0f, ~0u},
            SetCurrentCamera {0u},

            CreateTransform {1u, INVALID_UNIQUE_ID, {{1.0f, 1.0f}, Emergence::Math::PI / 6.0f, {2.0f, 3.0f}}},
            CreateSprite {1u, 1u, "Crate"_us, {{0.0f, 0.0f}, {1.0f, 1.0f}}, {0.5f, 0.5f}, 1u, ~0u},
        },
        AssetWaitPoint {},
        ScreenShotPoint {"TranslationRotationScale"_us},
    });
}

TEST_CASE (TranslationChange)
{
    ExecuteScenario ({
        TaskPoint {
            CreateTransform {0u, INVALID_UNIQUE_ID, {}},
            CreateCamera {0u, 2.0f, ~0u},
            SetCurrentCamera {0u},

            CreateTransform {1u, INVALID_UNIQUE_ID, {{10.0f, 0.0f}}},
            CreateSprite {1u, 1u, "Crate"_us, {{0.0f, 0.0f}, {1.0f, 1.0f}}, {0.5f, 0.5f}, 1u, ~0u},
        },
        AssetWaitPoint {},
        TaskPoint {
            UpdateTransform {1u, INVALID_UNIQUE_ID, {{1.0f, 0.0f}}},
        },
        ScreenShotPoint {"TranslationChange"_us},
    });
}

TEST_CASE (TransformHierarchy)
{
    ExecuteScenario ({
        TaskPoint {
            CreateTransform {0u, INVALID_UNIQUE_ID, {}},
            CreateCamera {0u, 2.0f, ~0u},
            SetCurrentCamera {0u},

            CreateTransform {1u, INVALID_UNIQUE_ID, {{-11.0f, 0.0f}}},
            CreateTransform {2u, 1u, {{10.0f, 0.0f}}},
            CreateSprite {2u, 1u, "Crate"_us, {{0.0f, 0.0f}, {1.0f, 1.0f}}, {0.5f, 0.5f}, 1u, ~0u},
        },
        AssetWaitPoint {},
        ScreenShotPoint {"TransformHierarchy"_us},
    });
}

TEST_CASE (TransformParentChange)
{
    ExecuteScenario ({
        TaskPoint {
            CreateTransform {0u, INVALID_UNIQUE_ID, {}},
            CreateCamera {0u, 2.0f, ~0u},
            SetCurrentCamera {0u},

            CreateTransform {1u, INVALID_UNIQUE_ID, {{-11.0f, 0.0f}}},
            CreateTransform {2u, 1u, {{10.0f, 0.0f}}},
            CreateSprite {2u, 1u, "Crate"_us, {{0.0f, 0.0f}, {1.0f, 1.0f}}, {0.5f, 0.5f}, 1u, ~0u},

            CreateTransform {3u, INVALID_UNIQUE_ID, {{-8.0f, 0.0f}}},
        },
        AssetWaitPoint {},
        TaskPoint {
            UpdateTransform {2u, 3u, {{10.0f, 0.0f}}},
        },
        ScreenShotPoint {"TransformParentChange"_us},
    });
}

END_SUITE
