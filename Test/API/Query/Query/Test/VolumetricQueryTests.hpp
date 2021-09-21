#pragma once

#include <Query/Test/Scenario.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Query::Test::VolumetricQuery
{
Scenario RayIntersections2D ();

Scenario ShapeIntersections2D ();

Scenario Duplicates ();

Scenario Edition ();

Scenario Intersections3D ();

Scenario MultipleSourcesEdition ();

Scenario MultipleSourcesDeletion ();

Scenario IntegerIntersections ();
} // namespace Emergence::Query::Test::VolumetricQuery

#define REGISTER_VOLUMETRIC_QUERY_TEST(Driver, TestName)                                                               \
    TEST_CASE (TestName)                                                                                               \
    {                                                                                                                  \
        Driver (Emergence::Query::Test::VolumetricQuery::TestName ());                                                 \
    }

#define REGISTER_ALL_VOLUMETRIC_QUERY_TESTS(Driver)                                                                    \
    REGISTER_VOLUMETRIC_QUERY_TEST (Driver, RayIntersections2D)                                                        \
    REGISTER_VOLUMETRIC_QUERY_TEST (Driver, ShapeIntersections2D)                                                      \
    REGISTER_VOLUMETRIC_QUERY_TEST (Driver, Duplicates)                                                                \
    REGISTER_VOLUMETRIC_QUERY_TEST (Driver, Edition)                                                                   \
    REGISTER_VOLUMETRIC_QUERY_TEST (Driver, Intersections3D)                                                           \
    REGISTER_VOLUMETRIC_QUERY_TEST (Driver, MultipleSourcesEdition)                                                    \
    REGISTER_VOLUMETRIC_QUERY_TEST (Driver, MultipleSourcesDeletion)                                                   \
    REGISTER_VOLUMETRIC_QUERY_TEST (Driver, IntegerIntersections)
