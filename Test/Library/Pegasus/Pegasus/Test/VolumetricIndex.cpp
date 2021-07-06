#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <Pegasus/Test/Record.hpp>
#include <Pegasus/Test/Scenario.hpp>
#include <Pegasus/Test/Shortcuts.hpp>

using namespace Emergence::Pegasus::Test;

namespace Constants
{
static const float minX = -100.0f;

static const float minY = -100.0f;

//static const float minZ = -100.0f;

static const float maxX = 100.0f;

static const float maxY = 100.0f;

//static const float maxZ = 100.0f;

static const float _m3 = -3.0f;

static const float _m1 = -1.0f;

static const float _0 = 0.0f;

static const float _2 = 2.0f;

static const float _7 = 7.0f;

static const float _8 = 8.0f;

static const float _8$5 = 8.5f;

static const float _9 = 9.0f;

static const float _10$5 = 10.5f;

static const float _50 = 50.0f;
};

static const std::vector <DimensionDescriptor> dimensions2d =
    {
        {
            Emergence::StandardLayout::ProjectNestedField (
                Record::Reflection::boundingBox, BoundingBox::Reflection::minX),
            &Constants::minX,

            Emergence::StandardLayout::ProjectNestedField (
                Record::Reflection::boundingBox, BoundingBox::Reflection::maxX),
            &Constants::maxX,
        },
        {
            Emergence::StandardLayout::ProjectNestedField (
                Record::Reflection::boundingBox, BoundingBox::Reflection::minY),
            &Constants::minY,

            Emergence::StandardLayout::ProjectNestedField (
                Record::Reflection::boundingBox, BoundingBox::Reflection::maxY),
            &Constants::maxY,
        }
    };

static const Record record_x10_11_y8_9_z4_5
    {
        0u,
        {
            "hugo"
        },
        {
            1u,
            0u,
        },
        {
            47u,
            96.0f,
        },
        {
            10.0f,
            8.0f,
            4.0f,
            11.0f,
            9.0f,
            5.0f,
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
            1u,
            0u,
        },
        {
            47u,
            96.0f,
        },
        {
            -2.0f,
            1.0f,
            0.0f,
            0.0f,
            4.0f,
            2.0f,
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
            1u,
            0u,
        },
        {
            47u,
            96.0f,
        },
        {
            15.0f,
            8.0f,
            50.0f,
            19.0f,
            11.0f,
            60.0f,
        },
        Record::Status::FLAG_ALIVE,
    };

BOOST_AUTO_TEST_SUITE (VolumetricIndex)

BOOST_DATA_TEST_CASE(
    TestRoutine, boost::unit_test::data::monomorphic::collection (
    std::vector <Scenario>
        {
            {
                Record::Reflection::GetMapping (),
                std::vector <Task>
                    {
                        CreateVolumetricIndex {"2d", dimensions2d},
                    } +
                Shortcuts::TestIsCanBeDropped ("2d")
            },
            {
                Record::Reflection::GetMapping (),
                std::vector <Task>
                    {
                        CreateVolumetricIndex {"2d", dimensions2d},
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
                                {"2d", "x = [8, 10.5], y = [7, 9]"},
                                {&Constants::_8, &Constants::_7}, {&Constants::_10$5, &Constants::_9}
                            }
                        },
                    VolumetricIndexShapeIntersectionLookupToEdit
                        {
                            {
                                {"2d", "x = [-3, -1], y = [0, 2]"},
                                {&Constants::_m3, &Constants::_m1}, {&Constants::_0, &Constants::_2}
                            }
                        },
                    &record_x10_11_y8_9_z4_5, nullptr, &record_xm2_0_y1_4_z0_2)
            },
            {
                Record::Reflection::GetMapping (),
                std::vector <Task>
                    {
                        CreateVolumetricIndex {"2d", dimensions2d},
                        OpenAllocator {},
                        AllocateAndInit {&record_x10_11_y8_9_z4_5},
                        AllocateAndInit {&record_xm2_0_y1_4_z0_2},
                        AllocateAndInit {&record_x15_19_y8_11_z50_60},
                        CloseAllocator {},
                    } +
                Shortcuts::TestCursorCopyAndMove (
                    VolumetricIndexRayIntersectionLookupToRead
                        {
                            {
                                {"2d", "origin = (7, 9), direction = (2, 0)"},
                                {&Constants::_7, &Constants::_9},
                                {&Constants::_2, &Constants::_0}
                            }
                        },
                    VolumetricIndexRayIntersectionLookupToEdit
                        {
                            {
                                {"2d", "origin = (-1, -3), direction = (0, 2)"},
                                {&Constants::_m1, &Constants::_m3}, {&Constants::_0, &Constants::_2}
                            }
                        },
                    &record_x10_11_y8_9_z4_5, &record_x15_19_y8_11_z50_60, &record_xm2_0_y1_4_z0_2)
            },
            {
                Record::Reflection::GetMapping (),
                {
                    OpenAllocator {},
                    AllocateAndInit {&record_x10_11_y8_9_z4_5},
                    AllocateAndInit {&record_xm2_0_y1_4_z0_2},
                    AllocateAndInit {&record_x15_19_y8_11_z50_60},
                    CloseAllocator {},
                    CreateVolumetricIndex {"2d", dimensions2d},

                    VolumetricIndexRayIntersectionLookupToRead
                        {
                            {
                                {"2d", "origin = (7, 9), direction = (2, 0)"},
                                {&Constants::_7, &Constants::_9}, {&Constants::_2, &Constants::_0}
                            }
                        },
                    CursorCheckAllOrdered
                        {
                            "origin = (7, 9), direction = (2, 0)",
                            {&record_x10_11_y8_9_z4_5, &record_x15_19_y8_11_z50_60}
                        },

                    VolumetricIndexShapeIntersectionLookupToRead
                        {
                            {
                                {"2d", "x = [-3, -1], y = [0, 2]"},
                                {&Constants::_m3, &Constants::_m1}, {&Constants::_0, &Constants::_2}
                            }
                        },
                    CursorCheckAllOrdered {"x = [-3, -1], y = [0, 2]", {&record_xm2_0_y1_4_z0_2}},
                },
            },
            {
                Record::Reflection::GetMapping (),
                {
                    CreateVolumetricIndex {"2d", dimensions2d},
                    OpenAllocator {},
                    AllocateAndInit {&record_x10_11_y8_9_z4_5},
                    CloseAllocator {},

                    VolumetricIndexRayIntersectionLookupToRead
                        {
                            {
                                {"2d", "origin = (7, 9), direction = (2, 0)"},
                                {&Constants::_7, &Constants::_9}, {&Constants::_2, &Constants::_0}
                            }
                        },
                    CursorCheckAllOrdered {"origin = (7, 9), direction = (2, 0)", {&record_x10_11_y8_9_z4_5}},

                    VolumetricIndexRayIntersectionLookupToRead
                        {
                            {
                                {"2d", "origin = (7, 8.5), direction = (2, 0)"},
                                {&Constants::_7, &Constants::_8$5}, {&Constants::_2, &Constants::_0}
                            }
                        },
                    CursorCheckAllOrdered {"origin = (7, 8.5), direction = (2, 0)", {&record_x10_11_y8_9_z4_5}},

                    VolumetricIndexRayIntersectionLookupToRead
                        {
                            {
                                {"2d", "origin = (7, 8), direction = (2, 0)"},
                                {&Constants::_7, &Constants::_8}, {&Constants::_2, &Constants::_0}
                            }
                        },
                    CursorCheckAllOrdered {"origin = (7, 8), direction = (2, 0)", {&record_x10_11_y8_9_z4_5}},

                    VolumetricIndexRayIntersectionLookupToRead
                        {
                            {
                                {"2d", "origin = (10.5, 8.5), direction = (2, 0)"},
                                {&Constants::_10$5, &Constants::_8$5}, {&Constants::_2, &Constants::_0}
                            }
                        },
                    CursorCheckAllOrdered {"origin = (10.5, 8.5), direction = (2, 0)", {&record_x10_11_y8_9_z4_5}},

                    VolumetricIndexRayIntersectionLookupToRead
                        {
                            {
                                {"2d", "origin = (7, 7), direction = (2, 0)"},
                                {&Constants::_7, &Constants::_7}, {&Constants::_2, &Constants::_0}
                            }
                        },
                    CursorCheckAllOrdered {"origin = (7, 7), direction = (2, 0)", {}},

                    VolumetricIndexRayIntersectionLookupToRead
                        {
                            {
                                {"2d", "origin = (10.5, 0), direction = (0, 2)"},
                                {&Constants::_10$5, &Constants::_0}, {&Constants::_0, &Constants::_2}
                            }
                        },
                    CursorCheckAllOrdered {"origin = (10.5, 0), direction = (0, 2)", {&record_x10_11_y8_9_z4_5}},

                    VolumetricIndexRayIntersectionLookupToRead
                        {
                            {
                                {"2d", "origin = (10.5, 50), direction = (0, 2)"},
                                {&Constants::_10$5, &Constants::_50}, {&Constants::_0, &Constants::_2}
                            }
                        },
                    CursorCheckAllOrdered {"origin = (10.5, 50), direction = (0, 2)", {}},

                    VolumetricIndexRayIntersectionLookupToRead
                        {
                            {
                                {"2d", "origin = (10.5, 0), direction = (0, -1)"},
                                {&Constants::_10$5, &Constants::_0}, {&Constants::_0, &Constants::_m1}
                            }
                        },
                    CursorCheckAllOrdered {"origin = (10.5, 0), direction = (0, -1)", {}},

                    VolumetricIndexRayIntersectionLookupToRead
                        {
                            {
                                {"2d", "origin = (10.5, 50), direction = (0, -1)"},
                                {&Constants::_10$5, &Constants::_50}, {&Constants::_0, &Constants::_m1}
                            }
                        },
                    CursorCheckAllOrdered {"origin = (10.5, 50), direction = (0, -1)", {&record_x10_11_y8_9_z4_5}},

                    VolumetricIndexRayIntersectionLookupToRead
                        {
                            {
                                {"2d", "origin = (7, 9), direction = (2, -1)"},
                                {&Constants::_7, &Constants::_9}, {&Constants::_2, &Constants::_m1}
                            }
                        },
                    CursorCheckAllOrdered {"origin = (7, 9), direction = (2, -1)", {}},

                    VolumetricIndexRayIntersectionLookupToRead
                        {
                            {
                                {"2d", "origin = (9, 9), direction = (2, -1)"},
                                {&Constants::_9, &Constants::_9}, {&Constants::_2, &Constants::_m1}
                            }
                        },
                    CursorCheckAllOrdered {"origin = (9, 9), direction = (2, -1)", {&record_x10_11_y8_9_z4_5}},
                },
            },
            {
                Record::Reflection::GetMapping (),
                {
                    CreateVolumetricIndex {"2d", dimensions2d},
                    OpenAllocator {},
                    AllocateAndInit {&record_x10_11_y8_9_z4_5},
                    CloseAllocator {},

                    VolumetricIndexShapeIntersectionLookupToRead
                        {
                            {
                                {"2d", "x = [8, 10.5], y = [7, 9]"},
                                {&Constants::_8, &Constants::_7}, {&Constants::_10$5, &Constants::_9}
                            }
                        },
                    CursorCheckAllOrdered {"x = [8, 10.5], y = [7, 9]", {&record_x10_11_y8_9_z4_5}},

                    VolumetricIndexShapeIntersectionLookupToRead
                        {
                            {
                                {"2d", "x = [8, 9], y = [7, 9]"},
                                {&Constants::_8, &Constants::_7}, {&Constants::_9, &Constants::_9}
                            }
                        },
                    CursorCheckAllOrdered {"x = [8, 9], y = [7, 9]", {}},

                    VolumetricIndexShapeIntersectionLookupToRead
                        {
                            {
                                {"2d", "x = [10.5, 10.5], y = [9, 9]"},
                                {&Constants::_10$5, &Constants::_9}, {&Constants::_10$5, &Constants::_9}
                            }
                        },
                    CursorCheckAllOrdered {"x = [10.5, 10.5], y = [9, 9]", {&record_x10_11_y8_9_z4_5}},

                    VolumetricIndexShapeIntersectionLookupToRead
                        {
                            {
                                {"2d", "x = [8, 10.5], y = [7, 8]"},
                                {&Constants::_8, &Constants::_7}, {&Constants::_10$5, &Constants::_8}
                            }
                        },
                    CursorCheckAllOrdered {"x = [8, 10.5], y = [7, 8]", {&record_x10_11_y8_9_z4_5}},

                    VolumetricIndexShapeIntersectionLookupToRead
                        {
                            {
                                {"2d", "x = [0, 50], y = [0, 50]"},
                                {&Constants::_0, &Constants::_0}, {&Constants::_50, &Constants::_50}
                            }
                        },
                    CursorCheckAllOrdered {"x = [0, 50], y = [0, 50]", {&record_x10_11_y8_9_z4_5}},

                    VolumetricIndexShapeIntersectionLookupToRead
                        {
                            {
                                {"2d", "x = [-3, 0], y = [0, 50]"},
                                {&Constants::_m3, &Constants::_0}, {&Constants::_0, &Constants::_50}
                            }
                        },
                    CursorCheckAllOrdered {"x = [-3, 0], y = [0, 50]", {}},
                }
            }
        }))
{
    sample.Execute ();
}

BOOST_AUTO_TEST_SUITE_END ()