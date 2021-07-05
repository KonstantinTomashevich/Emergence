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

static const float _9 = 9.0f;

static const float _10$5 = 10.5f;
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

static const Record entity_x10_11_y8_9_z4_5
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

static const Record entity_xm2_0_y1_4_z0_2
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

static const Record entity_x15_19_y8_11_z50_60
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
                        AllocateAndInit {&entity_x10_11_y8_9_z4_5},
                        AllocateAndInit {&entity_xm2_0_y1_4_z0_2},
                        AllocateAndInit {&entity_x15_19_y8_11_z50_60},
                        CloseAllocator {},
                    } +
                Shortcuts::TestCursorCopyAndMove (
                    VolumetricIndexShapeIntersectionLookupToRead
                        {
                            {
                                {"2d", "x = [8, 10.5], y = [7, 9]"},
                                {&Constants::_8, &Constants::_7},
                                {&Constants::_10$5, &Constants::_9}
                            }
                        },
                    VolumetricIndexShapeIntersectionLookupToEdit
                        {
                            {
                                {"2d", "x = [-3, -1], y = [0, 2]"},
                                {&Constants::_m3, &Constants::_m1},
                                {&Constants::_0, &Constants::_2}
                            }
                        },
                    &entity_x10_11_y8_9_z4_5, nullptr, &entity_xm2_0_y1_4_z0_2)
            },
            {
                Record::Reflection::GetMapping (),
                std::vector <Task>
                    {
                        CreateVolumetricIndex {"2d", dimensions2d},
                        OpenAllocator {},
                        AllocateAndInit {&entity_x10_11_y8_9_z4_5},
                        AllocateAndInit {&entity_xm2_0_y1_4_z0_2},
                        AllocateAndInit {&entity_x15_19_y8_11_z50_60},
                        CloseAllocator {},
                    } +
                Shortcuts::TestCursorCopyAndMove (
                    VolumetricIndexRayIntersectionLookupToRead
                        {
                            {
                                {"2d", "origin = (7, 8.5), direction = (2, 0)"},
                                {&Constants::_7, &Constants::_9},
                                {&Constants::_2, &Constants::_0}
                            }
                        },
                    VolumetricIndexRayIntersectionLookupToEdit
                        {
                            {
                                {"2d", "origin = (-1, -3), direction = (0, 2)"},
                                {&Constants::_m1, &Constants::_m3},
                                {&Constants::_0, &Constants::_2}
                            }
                        },
                    &entity_x10_11_y8_9_z4_5, &entity_x15_19_y8_11_z50_60, &entity_xm2_0_y1_4_z0_2)
            },
        }))
{
    sample.Execute ();
}

BOOST_AUTO_TEST_SUITE_END ()