#include <Assert/Assert.hpp>

#include <Query/Test/Data.hpp>
#include <Query/Test/VolumetricQueryTests.hpp>

namespace Emergence::Query::Test::VolumetricQuery
{
using namespace Tasks;

constexpr std::uint8_t FLAG_2D_SOURCE = 1u;

constexpr std::uint8_t FLAG_3D_SOURCE = 1u << 1u;

static Storage RequestBoundingBoxStorage (const Container::Vector<const void *> &_objects, std::uint8_t _sources)
{
    Storage storage {BoundingBox::Reflect ().mapping, _objects, {}};

    EMERGENCE_ASSERT (_sources > 0u);
    if (_sources & FLAG_2D_SOURCE)
    {
        storage.sources.emplace_back (
            Sources::Volumetric {"2d",
                                 {{-100.0f, BoundingBox::Reflect ().minX, 100.0f, BoundingBox::Reflect ().maxX},
                                  {-100.0f, BoundingBox::Reflect ().minY, 100.0f, BoundingBox::Reflect ().maxY}}});
    }

    if (_sources & FLAG_3D_SOURCE)
    {
        storage.sources.emplace_back (
            Sources::Volumetric {"3d",
                                 {
                                     {-100.0f, BoundingBox::Reflect ().minX, 100.0f, BoundingBox::Reflect ().maxX},
                                     {-100.0f, BoundingBox::Reflect ().minY, 100.0f, BoundingBox::Reflect ().maxY},
                                     {-100.0f, BoundingBox::Reflect ().minZ, 100.0f, BoundingBox::Reflect ().maxZ},
                                 }});
    }

    return storage;
}

static Storage RequestScreenRectStorage (const Container::Vector<const void *> &_objects)
{
    return {ScreenRect::Reflect ().mapping,
            _objects,
            {Sources::Volumetric {
                "screenRect",
                {
                    {int16_t (-4096), ScreenRect::Reflect ().minX, int16_t (4096), ScreenRect::Reflect ().maxX},
                    {int16_t (-4096), ScreenRect::Reflect ().minY, int16_t (4096), ScreenRect::Reflect ().maxY},
                }}}};
}

Scenario RayIntersections2D ()
{
    return {
        {
            RequestBoundingBoxStorage ({&BOX_MIN_10_8_4_MAX_11_9_5}, FLAG_2D_SOURCE),
        },
        {
            QueryRayIntersectionToRead {
                {{"2d", "origin = (7, -200), direction = (2, 0)"}, {7.0f, -200.0f}, {2.0f, 0.0f}}},
            CursorCheckAllOrdered {"origin = (7, -200), direction = (2, 0)", {}},

            QueryRayIntersectionToRead {{{"2d", "origin = (7, 9), direction = (2, 0)"}, {7.0f, 9.0f}, {2.0f, 0.0f}}},
            CursorCheckAllOrdered {"origin = (7, 9), direction = (2, 0)", {&BOX_MIN_10_8_4_MAX_11_9_5}},

            QueryRayIntersectionToRead {
                {{"2d", "origin = (7, 9), direction = (2, 0), distance = 1"}, {7.0f, 9.0f}, {2.0f, 0.0f}, 1.0f}},
            CursorCheckAllOrdered {"origin = (7, 9), direction = (2, 0), distance = 1", {}},

            QueryRayIntersectionToRead {
                {{"2d", "origin = (7, 9), direction = (2, 0), distance = 3"}, {7.0f, 9.0f}, {2.0f, 0.0f}, 3.0f}},
            CursorCheckAllOrdered {"origin = (7, 9), direction = (2, 0), distance = 3", {&BOX_MIN_10_8_4_MAX_11_9_5}},

            QueryRayIntersectionToRead {{{"2d", "origin = (7, 8.5), direction = (2, 0)"}, {7.0f, 8.5f}, {2.0f, 0.0f}}},
            CursorCheckAllOrdered {"origin = (7, 8.5), direction = (2, 0)", {&BOX_MIN_10_8_4_MAX_11_9_5}},

            QueryRayIntersectionToRead {{{"2d", "origin = (7, 8), direction = (2, 0)"}, {7.0f, 8.0f}, {2.0f, 0.0f}}},
            CursorCheckAllOrdered {"origin = (7, 8), direction = (2, 0)", {&BOX_MIN_10_8_4_MAX_11_9_5}},

            QueryRayIntersectionToRead {
                {{"2d", "origin = (10.5, 8.5), direction = (2, 0)"}, {10.5f, 8.5f}, {2.0f, 0.0f}}},
            CursorCheckAllOrdered {"origin = (10.5, 8.5), direction = (2, 0)", {&BOX_MIN_10_8_4_MAX_11_9_5}},

            QueryRayIntersectionToRead {{{"2d", "origin = (7, 7), direction = (2, 0)"}, {7.0f, 7.0f}, {2.0f, 0.0f}}},
            CursorCheckAllOrdered {"origin = (7, 7), direction = (2, 0)", {}},

            QueryRayIntersectionToRead {
                {{"2d", "origin = (10.5, 0), direction = (0, 2)"}, {10.5f, 0.0f}, {0.0f, 2.0f}}},
            CursorCheckAllOrdered {"origin = (10.5, 0), direction = (0, 2)", {&BOX_MIN_10_8_4_MAX_11_9_5}},

            QueryRayIntersectionToRead {
                {{"2d", "origin = (10.5, 50), direction = (0, 2)"}, {10.5f, 50.0f}, {0.0f, 2.0f}}},
            CursorCheckAllOrdered {"origin = (10.5, 50), direction = (0, 2)", {}},

            QueryRayIntersectionToRead {
                {{"2d", "origin = (10.5, 0), direction = (0, -1)"}, {10.5f, 0.0f}, {0.0f, -1.0f}}},
            CursorCheckAllOrdered {"origin = (10.5, 0), direction = (0, -1)", {}},

            QueryRayIntersectionToRead {
                {{"2d", "origin = (10.5, 50), direction = (0, -1)"}, {10.5f, 50.0f}, {0.0f, -1.0f}}},
            CursorCheckAllOrdered {"origin = (10.5, 50), direction = (0, -1)", {&BOX_MIN_10_8_4_MAX_11_9_5}},

            QueryRayIntersectionToRead {{{"2d", "origin = (7, 9), direction = (2, -1)"}, {7.0f, 9.0f}, {2.0f, -1.0f}}},
            CursorCheckAllOrdered {"origin = (7, 9), direction = (2, -1)", {}},

            QueryRayIntersectionToRead {{{"2d", "origin = (9, 9), direction = (2, -1)"}, {9.0f, 9.0f}, {2.0f, -1.0f}}},
            CursorCheckAllOrdered {"origin = (9, 9), direction = (2, -1)", {&BOX_MIN_10_8_4_MAX_11_9_5}},

            QueryRayIntersectionToRead {
                {{"2d", "origin = (9, 9), direction = (2, -1), distance = 1.1"}, {9.0f, 9.0f}, {2.0f, -1.0f}, 1.1f},
            },
            CursorCheckAllOrdered {"origin = (9, 9), direction = (2, -1), distance = 1.1", {}},

            QueryRayIntersectionToRead {
                {{"2d", "origin = (9, 9), direction = (2, -1), distance = 1.2"}, {9.0f, 9.0f}, {2.0f, -1.0f}, 1.2f},
            },
            CursorCheckAllOrdered {"origin = (9, 9), direction = (2, -1), distance = 1.2",
                                   {&BOX_MIN_10_8_4_MAX_11_9_5}},
        }};
}

Scenario ShapeIntersections2D ()
{
    return {
        {
            RequestBoundingBoxStorage ({&BOX_MIN_10_8_4_MAX_11_9_5}, FLAG_2D_SOURCE),
        },
        {
            QueryShapeIntersectionToRead {{{"2d", "min = (8, 7), max = (10.5, 9)"}, {8.0f, 7.0f}, {10.5f, 9.0f}}},
            CursorCheckAllOrdered {"min = (8, 7), max = (10.5, 9)", {&BOX_MIN_10_8_4_MAX_11_9_5}},

            QueryShapeIntersectionToRead {{{"2d", "min = (8, 7), max = (9, 9)"}, {8.0f, 7.0f}, {9.0f, 9.0f}}},
            CursorCheckAllOrdered {"min = (8, 7), max = (9, 9)", {}},

            QueryShapeIntersectionToRead {{{"2d", "min = (10.5, 9), max = (10.5, 9)"}, {10.5f, 9.0f}, {10.5f, 9.0f}}},
            CursorCheckAllOrdered {"min = (10.5, 9), max = (10.5, 9)", {&BOX_MIN_10_8_4_MAX_11_9_5}},

            QueryShapeIntersectionToRead {{{"2d", "min = (8, 7), max = (10.5, 8)"}, {8.0f, 7.0f}, {10.5f, 8.0f}}},
            CursorCheckAllOrdered {"min = (8, 7), max = (10.5, 8)", {&BOX_MIN_10_8_4_MAX_11_9_5}},

            QueryShapeIntersectionToRead {{{"2d", "min = (0, 0), max = (50, 50)"}, {0.0f, 0.0f}, {50.0f, 50.0f}}},
            CursorCheckAllOrdered {"min = (0, 0), max = (50, 50)", {&BOX_MIN_10_8_4_MAX_11_9_5}},

            QueryShapeIntersectionToRead {{{"2d", "min = (-3, 0), max = (0, 50)"}, {-3.0f, 0.0f}, {0.0f, 50.0f}}},
            CursorCheckAllOrdered {"min = (-3, 0), max = (0, 50)", {}},
        }};
}

Scenario Duplicates ()
{
    return {
        {
            RequestBoundingBoxStorage ({&BOX_MIN_10_8_4_MAX_11_9_5, &BOX_MIN_10_8_4_MAX_11_9_5,
                                        &BOX_MIN_M2_1_0_MAX_0_4_2, &BOX_MIN_10_8_4_MAX_11_9_5},
                                       FLAG_2D_SOURCE),
        },
        {
            QueryShapeIntersectionToRead {{{"2d", "min = (2, 2), max = (50, 50)"}, {2.0f, 2.0f}, {50.0f, 50.0f}}},
            CursorCheckAllOrdered {
                "min = (2, 2), max = (50, 50)",
                {&BOX_MIN_10_8_4_MAX_11_9_5, &BOX_MIN_10_8_4_MAX_11_9_5, &BOX_MIN_10_8_4_MAX_11_9_5}},

            QueryRayIntersectionToRead {{{"2d", "origin = (7, 9), direction = (2, 0)"}, {7.0f, 9.0f}, {2.0f, 0.0f}}},
            CursorCheckAllOrdered {
                "origin = (7, 9), direction = (2, 0)",
                {&BOX_MIN_10_8_4_MAX_11_9_5, &BOX_MIN_10_8_4_MAX_11_9_5, &BOX_MIN_10_8_4_MAX_11_9_5}},
        }};
}

Scenario Edition ()
{
    return {
        {
            RequestBoundingBoxStorage ({&BOX_MIN_10_8_4_MAX_11_9_5, &BOX_MIN_15_8_50_MAX_19_11_60}, FLAG_2D_SOURCE),
        },
        {
            QueryShapeIntersectionToEdit {{{"2d", "min = (2, 2), max = (10.5, 10.5)"}, {2.0f, 2.0f}, {10.5f, 10.5f}}},
            CursorCheck {"min = (2, 2), max = (10.5, 10.5)", &BOX_MIN_10_8_4_MAX_11_9_5},
            CursorEdit {"min = (2, 2), max = (10.5, 10.5)", &BOX_MIN_M2_1_0_MAX_0_4_2},
            CursorClose {"min = (2, 2), max = (10.5, 10.5)"},

            QueryShapeIntersectionToRead {{{"2d", "min = (2, 2), max = (10.5, 10.5)"}, {2.0f, 2.0f}, {10.5f, 10.5f}}},
            CursorCheckAllOrdered {"min = (2, 2), max = (10.5, 10.5)", {}},

            QueryShapeIntersectionToRead {{{"2d", "min = (-3, 0), max = (10.5, 10.5)"}, {-3.0f, 0.0f}, {10.5f, 10.5f}}},
            CursorCheckAllOrdered {"min = (-3, 0), max = (10.5, 10.5)", {&BOX_MIN_M2_1_0_MAX_0_4_2}},
        }};
}

Scenario Intersections3D ()
{
    return {
        {
            RequestBoundingBoxStorage (
                {&BOX_MIN_10_8_4_MAX_11_9_5, &BOX_MIN_M2_1_0_MAX_0_4_2, &BOX_MIN_15_8_50_MAX_19_11_60}, FLAG_3D_SOURCE),
        },
        {
            QueryShapeIntersectionToRead {
                {{"3d", "min = (-3, -3, -3), max = (50, 50, 50)"}, {-3.0f, -3.0f, -3.0f}, {50.0f, 50.0f, 50.0f}}},
            CursorCheckAllUnordered {
                "min = (-3, -3, -3), max = (50, 50, 50)",
                {&BOX_MIN_10_8_4_MAX_11_9_5, &BOX_MIN_M2_1_0_MAX_0_4_2, &BOX_MIN_15_8_50_MAX_19_11_60}},

            QueryShapeIntersectionToRead {
                {{"3d", "min = (9, 7, 0), max = (20, 10.5, 50)"}, {9.0f, 7.0f, 0.0f}, {20.0f, 10.5f, 50.0f}}},
            CursorCheckAllUnordered {"min = (9, 7, 0), max = (20, 10.5, 50)",
                                     {&BOX_MIN_10_8_4_MAX_11_9_5, &BOX_MIN_15_8_50_MAX_19_11_60}},

            QueryShapeIntersectionToRead {
                {{"3d", "min = (9, 7, 0), max = (20, 10.5, 20)"}, {9.0f, 7.0f, 0.0f}, {20.0f, 10.5f, 20.0f}}},
            CursorCheckAllOrdered {"min = (9, 7, 0), max = (20, 10.5, 20)", {&BOX_MIN_10_8_4_MAX_11_9_5}},

            QueryShapeIntersectionToRead {
                {{"3d", "min = (9, 7, 20), max = (20, 10.5, 50)"}, {9.0f, 7.0f, 20.0f}, {20.0f, 10.5f, 50.0f}}},
            CursorCheckAllOrdered {"min = (9, 7, 20), max = (20, 10.5, 50)", {&BOX_MIN_15_8_50_MAX_19_11_60}},

            QueryRayIntersectionToRead {
                {{"3d", "origin = (7, 8.5, 4.5), direction = (2, 0, 0)"}, {7.0f, 8.5f, 4.5f}, {2.0f, 0.0f, 0.0f}}},
            CursorCheckAllOrdered {"origin = (7, 8.5, 4.5), direction = (2, 0, 0)", {&BOX_MIN_10_8_4_MAX_11_9_5}},

            QueryRayIntersectionToRead {
                {{"3d", "origin = (7, 8.5, 4.5), direction = (2, 0, 9)"}, {7.0f, 8.5f, 4.5f}, {2.0f, 0.0f, 9.0f}}},
            CursorCheckAllOrdered {"origin = (7, 8.5, 4.5), direction = (2, 0, 9)", {&BOX_MIN_15_8_50_MAX_19_11_60}},

            QueryRayIntersectionToRead {{{"3d", "origin = (10.5, 8.5, 4.5), direction = (2, 0, 20)"},
                                         {10.5f, 8.5f, 4.5f},
                                         {2.0f, 0.0f, 20.0f}}},
            CursorCheckAllOrdered {"origin = (10.5, 8.5, 4.5), direction = (2, 0, 20)",
                                   {&BOX_MIN_10_8_4_MAX_11_9_5, &BOX_MIN_15_8_50_MAX_19_11_60}},

            QueryRayIntersectionToRead {{{"3d", "origin = (10.5, 8.5, 4.5), direction = (2, 0, 20), distance = 45.0"},
                                         {10.5f, 8.5f, 4.5f},
                                         {2.0f, 0.0f, 20.0f},
                                         45.0}},
            CursorCheckAllOrdered {"origin = (10.5, 8.5, 4.5), direction = (2, 0, 20), distance = 45.0",
                                   {&BOX_MIN_10_8_4_MAX_11_9_5}},
        }};
}

Scenario MultipleSourcesEdition ()
{
    return {
        {
            RequestBoundingBoxStorage ({&BOX_MIN_10_8_4_MAX_11_9_5, &BOX_MIN_15_8_50_MAX_19_11_60},
                                       FLAG_2D_SOURCE | FLAG_3D_SOURCE),
        },
        {
            QueryShapeIntersectionToRead {
                {{"3d", "min = (2, 2, 0), max = (10.5, 10.5, 4.5)"}, {2.0f, 2.0f, 0.0f}, {10.5f, 10.5f, 4.5f}}},
            CursorCheckAllOrdered {"min = (2, 2, 0), max = (10.5, 10.5, 4.5)", {&BOX_MIN_10_8_4_MAX_11_9_5}},
            CursorClose {"min = (2, 2, 0), max = (10.5, 10.5, 4.5)"},

            QueryShapeIntersectionToEdit {{{"2d", "min = (2, 2), max = (10.5, 10.5)"}, {2.0f, 2.0f}, {10.5f, 10.5f}}},
            CursorCheck {"min = (2, 2), max = (10.5, 10.5)", &BOX_MIN_10_8_4_MAX_11_9_5},
            CursorEdit {"min = (2, 2), max = (10.5, 10.5)", &BOX_MIN_M2_1_0_MAX_0_4_2},
            CursorIncrement {"min = (2, 2), max = (10.5, 10.5)"},
            CursorCheck {"min = (2, 2), max = (10.5, 10.5)", nullptr},
            CursorClose {"min = (2, 2), max = (10.5, 10.5)"},

            QueryShapeIntersectionToRead {{{"2d", "min = (2, 2), max = (10.5, 10.5)"}, {2.0f, 2.0f}, {10.5f, 10.5f}}},
            CursorCheckAllOrdered {"min = (2, 2), max = (10.5, 10.5)", {}},

            QueryShapeIntersectionToRead {{{"2d", "min = (-3, 0), max = (10.5, 10.5)"}, {-3.0f, 0.0f}, {10.5f, 10.5f}}},
            CursorCheckAllOrdered {"min = (-3, 0), max = (10.5, 10.5)", {&BOX_MIN_M2_1_0_MAX_0_4_2}},

            QueryShapeIntersectionToRead {
                {{"3d", "min = (2, 2, 0), max = (10.5, 10.5, 4.5)"}, {2.0f, 2.0f, 0.0f}, {10.5f, 10.5f, 4.5f}}},
            CursorCheckAllOrdered {"min = (2, 2, 0), max = (10.5, 10.5, 4.5)", {}},

            QueryShapeIntersectionToRead {
                {{"3d", "min = (-3, 0, 0), max = (10.5, 10.5, 4.5)"}, {-3.0f, 0.0f, 0.0f}, {10.5f, 10.5f, 4.5f}}},
            CursorCheckAllOrdered {"min = (-3, 0, 0), max = (10.5, 10.5, 4.5)", {&BOX_MIN_M2_1_0_MAX_0_4_2}},
        }};
}

Scenario MultipleSourcesDeletion ()
{
    return {
        {
            RequestBoundingBoxStorage ({&BOX_MIN_10_8_4_MAX_11_9_5, &BOX_MIN_15_8_50_MAX_19_11_60},
                                       FLAG_2D_SOURCE | FLAG_3D_SOURCE),
        },
        {
            QueryShapeIntersectionToRead {
                {{"3d", "min = (2, 2, 0), max = (10.5, 10.5, 4.5)"}, {2.0f, 2.0f, 0.0f}, {10.5f, 10.5f, 4.5f}}},
            CursorCheckAllOrdered {"min = (2, 2, 0), max = (10.5, 10.5, 4.5)", {&BOX_MIN_10_8_4_MAX_11_9_5}},
            CursorClose {"min = (2, 2, 0), max = (10.5, 10.5, 4.5)"},

            QueryShapeIntersectionToEdit {{{"2d", "min = (2, 2), max = (10.5, 10.5)"}, {2.0f, 2.0f}, {10.5f, 10.5f}}},
            CursorCheck {"min = (2, 2), max = (10.5, 10.5)", &BOX_MIN_10_8_4_MAX_11_9_5},
            CursorEdit {"min = (2, 2), max = (10.5, 10.5)", &BOX_MIN_M2_1_0_MAX_0_4_2},
            CursorDeleteObject {"min = (2, 2), max = (10.5, 10.5)"},
            CursorClose {"min = (2, 2), max = (10.5, 10.5)"},

            QueryShapeIntersectionToRead {{{"2d", "min = (-3, 2), max = (20, 10.5)"}, {-3.0f, 2.0f}, {20.0f, 10.5f}}},
            CursorCheckAllOrdered {"min = (-3, 2), max = (20, 10.5)", {&BOX_MIN_15_8_50_MAX_19_11_60}},

            QueryShapeIntersectionToRead {
                {{"3d", "min = (-3, 0, 0), max = (20, 10.5, 50)"}, {-3.0f, 0.0f, 0.0f}, {20.0f, 10.5f, 50.0f}}},
            CursorCheckAllOrdered {"min = (-3, 0, 0), max = (20, 10.5, 50)", {&BOX_MIN_15_8_50_MAX_19_11_60}},
        }};
}

Scenario IntegerIntersections ()
{
    return {{
                RequestScreenRectStorage (
                    {&RECT_MIN_0_0_MAX_500_200, &RECT_MIN_200_300_MAX_300_400, &RECT_MIN_700_400_MAX_1000_600}),
            },
            {
                QueryShapeIntersectionToRead {{{"screenRect", "min = (300, 300), max = (1000, 1000)"},
                                               {int16_t (300), int16_t (300)},
                                               {int16_t (1000), int16_t (1000)}}},
                CursorCheckAllUnordered {"min = (300, 300), max = (1000, 1000)",
                                         {&RECT_MIN_200_300_MAX_300_400, &RECT_MIN_700_400_MAX_1000_600}},

                QueryShapeIntersectionToRead {{{"screenRect", "min = (100, 100), max = (1000, 300)"},
                                               {int16_t (100), int16_t (100)},
                                               {int16_t (1000), int16_t (300)}}},
                CursorCheckAllUnordered {"min = (100, 100), max = (1000, 300)",
                                         {&RECT_MIN_0_0_MAX_500_200, &RECT_MIN_200_300_MAX_300_400}},

                QueryRayIntersectionToRead {{{"screenRect", "origin = (100, 250), direction = (100, 50)"},
                                             {int16_t (100), int16_t (250)},
                                             {int16_t (100), int16_t (50)}}},
                CursorCheckAllUnordered {"origin = (100, 250), direction = (100, 50)",
                                         {&RECT_MIN_200_300_MAX_300_400, &RECT_MIN_700_400_MAX_1000_600}},

                QueryRayIntersectionToRead {{{"screenRect", "origin = (300, 50), direction = (100, 100)"},
                                             {int16_t (300), int16_t (50)},
                                             {int16_t (100), int16_t (100)}}},
                CursorCheckAllUnordered {"origin = (300, 50), direction = (100, 100)",
                                         {&RECT_MIN_0_0_MAX_500_200, &RECT_MIN_700_400_MAX_1000_600}},
            }};
}
} // namespace Emergence::Query::Test::VolumetricQuery
