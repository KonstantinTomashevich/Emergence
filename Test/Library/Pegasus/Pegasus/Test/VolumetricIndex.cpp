#include <Pegasus/Test/Record.hpp>
#include <Pegasus/Test/Scenario.hpp>
#include <Pegasus/Test/Shortcuts.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Pegasus::Test;

static const float minX = -100.0f;

static const float minY = -100.0f;

static const float minZ = -100.0f;

static const float maxX = 100.0f;

static const float maxY = 100.0f;

static const float maxZ = 100.0f;

static const uint16_t screenRectMinX = 0u;

static const uint16_t screenRectMinY = 0u;

static const uint16_t screenRectMaxX = 4096u;

static const uint16_t screenRectMaxY = 4096u;

static const float f_m3 = -3.0f;

static const float f_m1 = -1.0f;

static const float f_0 = 0.0f;

static const float f_2 = 2.0f;

static const float f_4$5 = 4.5f;

static const float f_7 = 7.0f;

static const float f_8 = 8.0f;

static const float f_8$5 = 8.5f;

static const float f_9 = 9.0f;

static const float f_10$5 = 10.5f;

static const float f_20 = 20.0f;

static const float f_50 = 50.0f;

static const uint16_t u_50 = 50u;

static const uint16_t u_100 = 100u;

static const uint16_t u_250 = 250u;

static const uint16_t u_300 = 300u;

static const uint16_t u_1000 = 1000u;

// Dimensions should not be statically initialized, because there is no guaranty that reflection is ready.
static std::vector <DimensionDescriptor> GetDimensions2D ()
{
    return
        {
            {
                Emergence::StandardLayout::ProjectNestedField (
                    Record::Reflection::boundingBox, BoundingBox::Reflection::minX),
                &minX,

                Emergence::StandardLayout::ProjectNestedField (
                    Record::Reflection::boundingBox, BoundingBox::Reflection::maxX),
                &maxX,
            },
            {
                Emergence::StandardLayout::ProjectNestedField (
                    Record::Reflection::boundingBox, BoundingBox::Reflection::minY),
                &minY,

                Emergence::StandardLayout::ProjectNestedField (
                    Record::Reflection::boundingBox, BoundingBox::Reflection::maxY),
                &maxY,
            }
        };
}

// Dimensions should not be statically initialized, because there is no guaranty that reflection is ready.
static std::vector <DimensionDescriptor> GetDimensions3D ()
{
    std::vector <DimensionDescriptor> dimensions2d = GetDimensions2D ();
    return
        {
            dimensions2d[0u],
            dimensions2d[1u],
            {
                Emergence::StandardLayout::ProjectNestedField (
                    Record::Reflection::boundingBox, BoundingBox::Reflection::minZ),
                &minZ,

                Emergence::StandardLayout::ProjectNestedField (
                    Record::Reflection::boundingBox, BoundingBox::Reflection::maxZ),
                &maxZ,
            }
        };
}

// Dimensions should not be statically initialized, because there is no guaranty that reflection is ready.
static std::vector <DimensionDescriptor> GetDimensionsScreenRect ()
{
    return
        {
            {
                Emergence::StandardLayout::ProjectNestedField (
                    Record::Reflection::screenRect, ScreenRect::Reflection::minX),
                &screenRectMinX,

                Emergence::StandardLayout::ProjectNestedField (
                    Record::Reflection::screenRect, ScreenRect::Reflection::maxX),
                &screenRectMaxX,
            },
            {
                Emergence::StandardLayout::ProjectNestedField (
                    Record::Reflection::screenRect, ScreenRect::Reflection::minY),
                &screenRectMinY,

                Emergence::StandardLayout::ProjectNestedField (
                    Record::Reflection::screenRect, ScreenRect::Reflection::maxY),
                &screenRectMaxY,
            }
        };
}

static const Record record_x10_11_y8_9_z4_5
    {
        0u,
        {
            "hugo"
        },
        {
            10.0f,
            8.0f,
            4.0f,
            11.0f,
            9.0f,
            5.0f,
        },
        {
            200u,
            300u,
            300u,
            400u,
        },
        Record::Status::FLAG_ALIVE,
    };

static const Record record_xm2_0_y1_4_z0_2
    {
        1u,
        {
            "karl"
        },
        {
            -2.0f,
            1.0f,
            0.0f,
            0.0f,
            4.0f,
            2.0f,
        },
        {
            0u,
            0u,
            500u,
            200u,
        },
        Record::Status::FLAG_ALIVE,
    };

static const Record record_x15_19_y8_11_z50_60
    {
        2u,
        {
            "xavier"
        },
        {
            15.0f,
            8.0f,
            50.0f,
            19.0f,
            11.0f,
            60.0f,
        },
        {
            700u,
            400u,
            1000u,
            600u,
        },
        Record::Status::FLAG_ALIVE,
    };

BEGIN_SUITE (VolumetricIndex)

TEST_CASE (ReferenceManipulations)
{
    Scenario {
        Record::Reflection::GetMapping (),
        std::vector <Task>
            {
                CreateVolumetricIndex {"2d", GetDimensions2D ()},
            } +
        Shortcuts::TestIsCanBeDropped ("2d")
    };
}

TEST_CASE (ShapeIntersectionCursorManipulations)
{
    Scenario {
        Record::Reflection::GetMapping (),
        std::vector <Task>
            {
                CreateVolumetricIndex {"2d", GetDimensions2D ()},
                OpenAllocator {},
                AllocateAndInit {&record_x10_11_y8_9_z4_5},
                AllocateAndInit {&record_xm2_0_y1_4_z0_2},
                AllocateAndInit {&record_x15_19_y8_11_z50_60},
                CloseAllocator {},
            } +
        Shortcuts::TestCursorCopyAndMove (
            VolumetricIndexShapeIntersectionLookupToRead
                {
                    {
                        {"2d", "min = (8, 7), max = (10.5, 9)"},
                        {&f_8, &f_7}, {&f_10$5, &f_9}
                    }
                },
            VolumetricIndexShapeIntersectionLookupToEdit
                {
                    {
                        {"2d", "min = (-3, 0), max = (-1, 2)"},
                        {&f_m3, &f_m1}, {&f_0, &f_2}
                    }
                },
            &record_x10_11_y8_9_z4_5, nullptr, &record_xm2_0_y1_4_z0_2)
    };
}

TEST_CASE (RayIntersectionCursorManipulations)
{
    Scenario {
        Record::Reflection::GetMapping (),
        std::vector <Task>
            {
                CreateVolumetricIndex {"2d", GetDimensions2D ()},
                OpenAllocator {},
                AllocateAndInit {&record_x10_11_y8_9_z4_5},
                AllocateAndInit {&record_xm2_0_y1_4_z0_2},
                AllocateAndInit {&record_x15_19_y8_11_z50_60},
                CloseAllocator {},
            } +
        Shortcuts::TestCursorCopyAndMove (
            VolumetricIndexRayIntersectionLookupToRead
                {
                    {{"2d", "origin = (7, 9), direction = (2, 0)"}, {&f_7, &f_9}, {&f_2, &f_0}}
                },
            VolumetricIndexRayIntersectionLookupToEdit
                {
                    {{"2d", "origin = (-1, -3), direction = (0, 2)"}, {&f_m1, &f_m3}, {&f_0, &f_2}}
                },
            &record_x10_11_y8_9_z4_5, &record_x15_19_y8_11_z50_60, &record_xm2_0_y1_4_z0_2)
    };
}

TEST_CASE (InsertBeforeCreation)
{
    Scenario {
        Record::Reflection::GetMapping (),
        {
            OpenAllocator {},
            AllocateAndInit {&record_x10_11_y8_9_z4_5},
            AllocateAndInit {&record_xm2_0_y1_4_z0_2},
            AllocateAndInit {&record_x15_19_y8_11_z50_60},
            CloseAllocator {},
            CreateVolumetricIndex {"2d", GetDimensions2D ()},

            VolumetricIndexRayIntersectionLookupToRead
                {
                    {{"2d", "origin = (7, 9), direction = (2, 0)"}, {&f_7, &f_9}, {&f_2, &f_0}}
                },
            CursorCheckAllOrdered
                {
                    "origin = (7, 9), direction = (2, 0)",
                    {&record_x10_11_y8_9_z4_5, &record_x15_19_y8_11_z50_60}
                },

            VolumetricIndexShapeIntersectionLookupToRead
                {
                    {{"2d", "min = (-3, 0), max = (-1, 2)"}, {&f_m3, &f_m1}, {&f_0, &f_2}}
                },
            CursorCheckAllOrdered {"min = (-3, 0), max = (-1, 2)", {&record_xm2_0_y1_4_z0_2}},
        },
    };
}

TEST_CASE (RayIntersections2D)
{
    Scenario {
        Record::Reflection::GetMapping (),
        {
            CreateVolumetricIndex {"2d", GetDimensions2D ()},
            OpenAllocator {},
            AllocateAndInit {&record_x10_11_y8_9_z4_5},
            CloseAllocator {},

            VolumetricIndexRayIntersectionLookupToRead
                {
                    {
                        {"2d", "origin = (7, 9), direction = (2, 0)"},
                        {&f_7, &f_9}, {&f_2, &f_0}
                    }
                },
            CursorCheckAllOrdered {"origin = (7, 9), direction = (2, 0)", {&record_x10_11_y8_9_z4_5}},

            VolumetricIndexRayIntersectionLookupToRead
                {
                    {{"2d", "origin = (7, 8.5), direction = (2, 0)"}, {&f_7, &f_8$5}, {&f_2, &f_0}}
                },
            CursorCheckAllOrdered {"origin = (7, 8.5), direction = (2, 0)", {&record_x10_11_y8_9_z4_5}},

            VolumetricIndexRayIntersectionLookupToRead
                {
                    {{"2d", "origin = (7, 8), direction = (2, 0)"}, {&f_7, &f_8}, {&f_2, &f_0}}
                },
            CursorCheckAllOrdered {"origin = (7, 8), direction = (2, 0)", {&record_x10_11_y8_9_z4_5}},

            VolumetricIndexRayIntersectionLookupToRead
                {
                    {{"2d", "origin = (10.5, 8.5), direction = (2, 0)"}, {&f_10$5, &f_8$5}, {&f_2, &f_0}}
                },
            CursorCheckAllOrdered {"origin = (10.5, 8.5), direction = (2, 0)", {&record_x10_11_y8_9_z4_5}},

            VolumetricIndexRayIntersectionLookupToRead
                {
                    {{"2d", "origin = (7, 7), direction = (2, 0)"}, {&f_7, &f_7}, {&f_2, &f_0}}
                },
            CursorCheckAllOrdered {"origin = (7, 7), direction = (2, 0)", {}},

            VolumetricIndexRayIntersectionLookupToRead
                {
                    {{"2d", "origin = (10.5, 0), direction = (0, 2)"}, {&f_10$5, &f_0}, {&f_0, &f_2}}
                },
            CursorCheckAllOrdered {"origin = (10.5, 0), direction = (0, 2)", {&record_x10_11_y8_9_z4_5}},

            VolumetricIndexRayIntersectionLookupToRead
                {
                    {{"2d", "origin = (10.5, 50), direction = (0, 2)"}, {&f_10$5, &f_50}, {&f_0, &f_2}}
                },
            CursorCheckAllOrdered {"origin = (10.5, 50), direction = (0, 2)", {}},

            VolumetricIndexRayIntersectionLookupToRead
                {
                    {{"2d", "origin = (10.5, 0), direction = (0, -1)"}, {&f_10$5, &f_0}, {&f_0, &f_m1}}
                },
            CursorCheckAllOrdered {"origin = (10.5, 0), direction = (0, -1)", {}},

            VolumetricIndexRayIntersectionLookupToRead
                {
                    {{"2d", "origin = (10.5, 50), direction = (0, -1)"}, {&f_10$5, &f_50}, {&f_0, &f_m1}}
                },
            CursorCheckAllOrdered {"origin = (10.5, 50), direction = (0, -1)", {&record_x10_11_y8_9_z4_5}},

            VolumetricIndexRayIntersectionLookupToRead
                {
                    {{"2d", "origin = (7, 9), direction = (2, -1)"}, {&f_7, &f_9}, {&f_2, &f_m1}}
                },
            CursorCheckAllOrdered {"origin = (7, 9), direction = (2, -1)", {}},

            VolumetricIndexRayIntersectionLookupToRead
                {
                    {{"2d", "origin = (9, 9), direction = (2, -1)"}, {&f_9, &f_9}, {&f_2, &f_m1}}
                },
            CursorCheckAllOrdered {"origin = (9, 9), direction = (2, -1)", {&record_x10_11_y8_9_z4_5}},
        },
    };
}

TEST_CASE (ShapeIntersections2D)
{
    Scenario {
        Record::Reflection::GetMapping (),
        {
            CreateVolumetricIndex {"2d", GetDimensions2D ()},
            OpenAllocator {},
            AllocateAndInit {&record_x10_11_y8_9_z4_5},
            CloseAllocator {},

            VolumetricIndexShapeIntersectionLookupToRead
                {
                    {{"2d", "min = (8, 7), max = (10.5, 9)"}, {&f_8, &f_7}, {&f_10$5, &f_9}}
                },
            CursorCheckAllOrdered {"min = (8, 7), max = (10.5, 9)", {&record_x10_11_y8_9_z4_5}},

            VolumetricIndexShapeIntersectionLookupToRead
                {
                    {{"2d", "min = (8, 7), max = (9, 9)"}, {&f_8, &f_7}, {&f_9, &f_9}}
                },
            CursorCheckAllOrdered {"min = (8, 7), max = (9, 9)", {}},

            VolumetricIndexShapeIntersectionLookupToRead
                {
                    {{"2d", "min = (10.5, 9), max = (10.5, 9)"}, {&f_10$5, &f_9}, {&f_10$5, &f_9}}
                },
            CursorCheckAllOrdered {"min = (10.5, 9), max = (10.5, 9)", {&record_x10_11_y8_9_z4_5}},

            VolumetricIndexShapeIntersectionLookupToRead
                {
                    {{"2d", "min = (8, 7), max = (10.5, 8)"}, {&f_8, &f_7}, {&f_10$5, &f_8}}
                },
            CursorCheckAllOrdered {"min = (8, 7), max = (10.5, 8)", {&record_x10_11_y8_9_z4_5}},

            VolumetricIndexShapeIntersectionLookupToRead
                {
                    {{"2d", "min = (0, 0), max = (50, 50)"}, {&f_0, &f_0}, {&f_50, &f_50}}
                },
            CursorCheckAllOrdered {"min = (0, 0), max = (50, 50)", {&record_x10_11_y8_9_z4_5}},

            VolumetricIndexShapeIntersectionLookupToRead
                {
                    {{"2d", "min = (-3, 0), max = (0, 50)"}, {&f_m3, &f_0}, {&f_0, &f_50}}
                },
            CursorCheckAllOrdered {"min = (-3, 0), max = (0, 50)", {}},
        }
    };
}

TEST_CASE (Dupicates)
{
    Scenario {
        Record::Reflection::GetMapping (),
        {
            CreateVolumetricIndex {"2d", GetDimensions2D ()},
            OpenAllocator {},
            AllocateAndInit {&record_x10_11_y8_9_z4_5},
            AllocateAndInit {&record_x10_11_y8_9_z4_5},
            AllocateAndInit {&record_xm2_0_y1_4_z0_2},
            AllocateAndInit {&record_x10_11_y8_9_z4_5},
            CloseAllocator {},

            VolumetricIndexShapeIntersectionLookupToRead
                {
                    {{"2d", "min = (2, 2), max = (50, 50)"}, {&f_2, &f_2}, {&f_50, &f_50}}
                },
            CursorCheckAllOrdered
                {
                    "min = (2, 2), max = (50, 50)",
                    {&record_x10_11_y8_9_z4_5, &record_x10_11_y8_9_z4_5, &record_x10_11_y8_9_z4_5}
                },

            VolumetricIndexRayIntersectionLookupToRead
                {
                    {{"2d", "origin = (7, 9), direction = (2, 0)"}, {&f_7, &f_9}, {&f_2, &f_0}}
                },
            CursorCheckAllOrdered
                {
                    "origin = (7, 9), direction = (2, 0)",
                    {&record_x10_11_y8_9_z4_5, &record_x10_11_y8_9_z4_5, &record_x10_11_y8_9_z4_5}
                },
        }
    };
}

TEST_CASE (Edition)
{
    Scenario {
        Record::Reflection::GetMapping (),
        {
            CreateVolumetricIndex {"2d", GetDimensions2D ()},
            OpenAllocator {},
            AllocateAndInit {&record_x10_11_y8_9_z4_5},
            AllocateAndInit {&record_x15_19_y8_11_z50_60},
            CloseAllocator {},

            VolumetricIndexShapeIntersectionLookupToEdit
                {
                    {{"2d", "min = (2, 2), max = (10.5, 10.5)"}, {&f_2, &f_2}, {&f_10$5, &f_10$5}}
                },
            CursorCheck {"min = (2, 2), max = (10.5, 10.5)", &record_x10_11_y8_9_z4_5},
            CursorEdit {"min = (2, 2), max = (10.5, 10.5)", &record_xm2_0_y1_4_z0_2},
            CloseCursor {"min = (2, 2), max = (10.5, 10.5)"},

            VolumetricIndexShapeIntersectionLookupToRead
                {
                    {{"2d", "min = (2, 2), max = (10.5, 10.5)"}, {&f_2, &f_2}, {&f_10$5, &f_10$5}}
                },
            CursorCheckAllOrdered {"min = (2, 2), max = (10.5, 10.5)", {}},

            VolumetricIndexShapeIntersectionLookupToRead
                {
                    {{"2d", "min = (-3, 0), max = (10.5, 10.5)"}, {&f_m3, &f_0}, {&f_10$5, &f_10$5}}
                },
            CursorCheckAllOrdered {"min = (-3, 0), max = (10.5, 10.5)", {&record_xm2_0_y1_4_z0_2}},
        }
    };
}

TEST_CASE (Intersections3D)
{
    Scenario {
        Record::Reflection::GetMapping (),
        {
            CreateVolumetricIndex {"3d", GetDimensions3D ()},
            OpenAllocator {},
            AllocateAndInit {&record_x10_11_y8_9_z4_5},
            AllocateAndInit {&record_xm2_0_y1_4_z0_2},
            AllocateAndInit {&record_x15_19_y8_11_z50_60},
            CloseAllocator {},

            VolumetricIndexShapeIntersectionLookupToRead
                {
                    {
                        {"3d", "min = (-3, -3, -3), max = (50, 50, 50)"},
                        {&f_m3, &f_m3, &f_m3}, {&f_50, &f_50, &f_50}
                    }
                },
            CursorCheckAllUnordered
                {
                    "min = (-3, -3, -3), max = (50, 50, 50)",
                    {&record_x10_11_y8_9_z4_5, &record_xm2_0_y1_4_z0_2, &record_x15_19_y8_11_z50_60}
                },

            VolumetricIndexShapeIntersectionLookupToRead
                {
                    {
                        {"3d", "min = (9, 7, 0), max = (20, 10.5, 50)"},
                        {&f_9, &f_7, &f_0}, {&f_20, &f_10$5, &f_50}
                    }
                },
            CursorCheckAllUnordered
                {
                    "min = (9, 7, 0), max = (20, 10.5, 50)",
                    {&record_x10_11_y8_9_z4_5, &record_x15_19_y8_11_z50_60}
                },

            VolumetricIndexShapeIntersectionLookupToRead
                {
                    {
                        {"3d", "min = (9, 7, 0), max = (20, 10.5, 20)"},
                        {&f_9, &f_7, &f_0}, {&f_20, &f_10$5, &f_20}
                    }
                },
            CursorCheckAllOrdered {"min = (9, 7, 0), max = (20, 10.5, 20)", {&record_x10_11_y8_9_z4_5}},

            VolumetricIndexShapeIntersectionLookupToRead
                {
                    {
                        {"3d", "min = (9, 7, 20), max = (20, 10.5, 50)"},
                        {&f_9, &f_7, &f_20}, {&f_20, &f_10$5, &f_50}
                    }
                },
            CursorCheckAllOrdered {"min = (9, 7, 20), max = (20, 10.5, 50)", {&record_x15_19_y8_11_z50_60}},

            VolumetricIndexRayIntersectionLookupToRead
                {
                    {
                        {"3d", "origin = (7, 8.5, 4.5), direction = (2, 0, 0)"},
                        {&f_7, &f_8$5, &f_4$5}, {&f_2, &f_0, &f_0}
                    }
                },
            CursorCheckAllOrdered {"origin = (7, 8.5, 4.5), direction = (2, 0, 0)", {&record_x10_11_y8_9_z4_5}},

            VolumetricIndexRayIntersectionLookupToRead
                {
                    {
                        {"3d", "origin = (7, 8.5, 4.5), direction = (2, 0, 9)"},
                        {&f_7, &f_8$5, &f_4$5}, {&f_2, &f_0, &f_9}
                    }
                },
            CursorCheckAllOrdered
                {
                    "origin = (7, 8.5, 4.5), direction = (2, 0, 9)",
                    {&record_x15_19_y8_11_z50_60}
                },

            VolumetricIndexRayIntersectionLookupToRead
                {
                    {
                        {"3d", "origin = (10.5, 8.5, 4.5), direction = (2, 0, 20)"},
                        {&f_10$5, &f_8$5, &f_4$5}, {&f_2, &f_0, &f_20}
                    }
                },
            CursorCheckAllOrdered
                {
                    "origin = (10.5, 8.5, 4.5), direction = (2, 0, 20)",
                    {&record_x10_11_y8_9_z4_5, &record_x15_19_y8_11_z50_60}
                },
        }
    };
}

TEST_CASE (MultipleIndicesEdition)
{
    Scenario {
        Record::Reflection::GetMapping (),
        {
            CreateVolumetricIndex {"2d", GetDimensions2D ()},
            CreateVolumetricIndex {"3d", GetDimensions3D ()},
            OpenAllocator {},
            AllocateAndInit {&record_x10_11_y8_9_z4_5},
            AllocateAndInit {&record_x15_19_y8_11_z50_60},
            CloseAllocator {},

            VolumetricIndexShapeIntersectionLookupToRead
                {
                    {
                        {"3d", "min = (2, 2, 0), max = (10.5, 10.5, 4.5)"},
                        {&f_2, &f_2, &f_0}, {&f_10$5, &f_10$5, &f_4$5}
                    }
                },
            CursorCheckAllOrdered {"min = (2, 2, 0), max = (10.5, 10.5, 4.5)", {&record_x10_11_y8_9_z4_5}},
            CloseCursor {"min = (2, 2, 0), max = (10.5, 10.5, 4.5)"},

            VolumetricIndexShapeIntersectionLookupToEdit
                {
                    {
                        {"2d", "min = (2, 2), max = (10.5, 10.5)"},
                        {&f_2, &f_2}, {&f_10$5, &f_10$5}
                    }
                },
            CursorCheck {"min = (2, 2), max = (10.5, 10.5)", &record_x10_11_y8_9_z4_5},
            CursorEdit {"min = (2, 2), max = (10.5, 10.5)", &record_xm2_0_y1_4_z0_2},
            CloseCursor {"min = (2, 2), max = (10.5, 10.5)"},

            VolumetricIndexShapeIntersectionLookupToRead
                {
                    {
                        {"2d", "min = (2, 2), max = (10.5, 10.5)"},
                        {&f_2, &f_2}, {&f_10$5, &f_10$5}
                    }
                },
            CursorCheckAllOrdered {"min = (2, 2), max = (10.5, 10.5)", {}},

            VolumetricIndexShapeIntersectionLookupToRead
                {
                    {
                        {"2d", "min = (-3, 0), max = (10.5, 10.5)"},
                        {&f_m3, &f_0}, {&f_10$5, &f_10$5}
                    }
                },
            CursorCheckAllOrdered {"min = (-3, 0), max = (10.5, 10.5)", {&record_xm2_0_y1_4_z0_2}},

            VolumetricIndexShapeIntersectionLookupToRead
                {
                    {
                        {"3d", "min = (2, 2, 0), max = (10.5, 10.5, 4.5)"},
                        {&f_2, &f_2, &f_0}, {&f_10$5, &f_10$5, &f_4$5}
                    }
                },
            CursorCheckAllOrdered {"min = (2, 2, 0), max = (10.5, 10.5, 4.5)", {}},

            VolumetricIndexShapeIntersectionLookupToRead
                {
                    {
                        {"3d", "min = (-3, 0, 0), max = (10.5, 10.5, 4.5)"},
                        {&f_m3, &f_0, &f_0}, {&f_10$5, &f_10$5, &f_4$5}
                    }
                },
            CursorCheckAllOrdered {"min = (-3, 0, 0), max = (10.5, 10.5, 4.5)", {&record_xm2_0_y1_4_z0_2}},
        }
    };
}

TEST_CASE (MultipleIndicesDeletion)
{
    Scenario {
        Record::Reflection::GetMapping (),
        {
            CreateVolumetricIndex {"2d", GetDimensions2D ()},
            CreateVolumetricIndex {"3d", GetDimensions3D ()},
            OpenAllocator {},
            AllocateAndInit {&record_x10_11_y8_9_z4_5},
            AllocateAndInit {&record_x15_19_y8_11_z50_60},
            CloseAllocator {},

            VolumetricIndexShapeIntersectionLookupToRead
                {
                    {
                        {"3d", "min = (2, 2, 0), max = (10.5, 10.5, 4.5)"},
                        {&f_2, &f_2, &f_0}, {&f_10$5, &f_10$5, &f_4$5}
                    }
                },
            CursorCheckAllOrdered {"min = (2, 2, 0), max = (10.5, 10.5, 4.5)", {&record_x10_11_y8_9_z4_5}},
            CloseCursor {"min = (2, 2, 0), max = (10.5, 10.5, 4.5)"},

            VolumetricIndexShapeIntersectionLookupToEdit
                {
                    {
                        {"2d", "min = (2, 2), max = (10.5, 10.5)"},
                        {&f_2, &f_2}, {&f_10$5, &f_10$5}
                    }
                },
            CursorCheck {"min = (2, 2), max = (10.5, 10.5)", &record_x10_11_y8_9_z4_5},
            CursorEdit {"min = (2, 2), max = (10.5, 10.5)", &record_xm2_0_y1_4_z0_2},
            CursorDeleteRecord {"min = (2, 2), max = (10.5, 10.5)"},
            CloseCursor {"min = (2, 2), max = (10.5, 10.5)"},

            VolumetricIndexShapeIntersectionLookupToRead
                {
                    {
                        {"2d", "min = (-3, 2), max = (20, 10.5)"},
                        {&f_m3, &f_2}, {&f_20, &f_10$5}
                    }
                },
            CursorCheckAllOrdered {"min = (-3, 2), max = (20, 10.5)", {&record_x15_19_y8_11_z50_60}},

            VolumetricIndexShapeIntersectionLookupToRead
                {
                    {
                        {"3d", "min = (-3, 0, 0), max = (20, 10.5, 50)"},
                        {&f_m3, &f_0, &f_0}, {&f_20, &f_10$5, &f_50}
                    }
                },
            CursorCheckAllOrdered {"min = (-3, 0, 0), max = (20, 10.5, 50)", {&record_x15_19_y8_11_z50_60}},
        }
    };
}

TEST_CASE (IntegerIntersections)
{
    Scenario {
        Record::Reflection::GetMapping (),
        {
            CreateVolumetricIndex {"screenRect", GetDimensionsScreenRect ()},
            OpenAllocator {},
            AllocateAndInit {&record_x10_11_y8_9_z4_5},
            AllocateAndInit {&record_xm2_0_y1_4_z0_2},
            AllocateAndInit {&record_x15_19_y8_11_z50_60},
            CloseAllocator {},

            VolumetricIndexShapeIntersectionLookupToRead
                {
                    {
                        {"screenRect", "min = (300, 300), max = (1000, 1000)"},
                        {&u_300, &u_300}, {&u_1000, &u_1000}
                    }
                },
            CursorCheckAllUnordered
                {
                    "min = (300, 300), max = (1000, 1000)",
                    {&record_x10_11_y8_9_z4_5, &record_x15_19_y8_11_z50_60}
                },

            VolumetricIndexShapeIntersectionLookupToRead
                {
                    {
                        {"screenRect", "min = (100, 100), max = (1000, 300)"},
                        {&u_100, &u_100}, {&u_1000, &u_300}
                    }
                },
            CursorCheckAllUnordered
                {
                    "min = (100, 100), max = (1000, 300)",
                    {&record_x10_11_y8_9_z4_5, &record_xm2_0_y1_4_z0_2}
                },

            VolumetricIndexRayIntersectionLookupToRead
                {
                    {
                        {"screenRect", "origin = (100, 250), direction = (100, 50)"},
                        {&u_100, &u_250}, {&u_100, &u_50}
                    }
                },
            CursorCheckAllOrdered
                {
                    "origin = (100, 250), direction = (100, 50)",
                    {&record_x10_11_y8_9_z4_5, &record_x15_19_y8_11_z50_60}
                },

            VolumetricIndexRayIntersectionLookupToRead
                {
                    {
                        {"screenRect", "origin = (300, 50), direction = (100, 100)"},
                        {&u_300, &u_50}, {&u_100, &u_100}
                    }
                },
            CursorCheckAllOrdered
                {
                    "origin = (300, 50), direction = (100, 100)",
                    {&record_xm2_0_y1_4_z0_2, &record_x15_19_y8_11_z50_60}
                },
        }
    };
}

END_SUITE