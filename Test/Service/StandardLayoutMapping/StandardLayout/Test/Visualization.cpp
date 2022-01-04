#include <StandardLayout/Test/Visualization.hpp>
#include <StandardLayout/Visualization.hpp>

#include <StandardLayout/MappingRegistration.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::StandardLayout::Test
{
bool VisualizationTestIncludeMarker () noexcept
{
    return true;
}

struct NoNesting final
{
    struct Reflection final
    {
        FieldId x;
        FieldId y;
        FieldId z;

        Mapping mapping;
    };

    static const Reflection &Reflect ();

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct OneNestingLevel final
{
    struct Reflection final
    {
        FieldId first;
        FieldId second;

        Mapping mapping;
    };

    static const Reflection &Reflect ();

    NoNesting first;
    NoNesting second;
};

struct TwoNestingLevels final
{
    struct Reflection final
    {
        FieldId first;
        FieldId second;

        Mapping mapping;
    };

    static const Reflection &Reflect ();

    OneNestingLevel first;
    NoNesting second;
};

const NoNesting::Reflection &NoNesting::Reflect ()
{
    static Reflection reflection = [] () -> Reflection
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (NoNesting)

        EMERGENCE_MAPPING_REGISTER_REGULAR (x)
        EMERGENCE_MAPPING_REGISTER_REGULAR (y)
        EMERGENCE_MAPPING_REGISTER_REGULAR (z)

        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}

const OneNestingLevel::Reflection &OneNestingLevel::Reflect ()
{
    static Reflection reflection = [] () -> Reflection
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (OneNestingLevel)

        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (first)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (second)

        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}

const TwoNestingLevels::Reflection &TwoNestingLevels::Reflect ()
{
    static Reflection reflection = [] () -> Reflection
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (TwoNestingLevels)

        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (first)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (second)

        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}
} // namespace Emergence::StandardLayout::Test

BEGIN_SUITE (MappingVisualization)

TEST_CASE (NoNesting)
{
    using namespace Emergence::VisualGraph;
    using namespace Emergence::StandardLayout;

    const Graph result = Visualization::GraphFromMapping (Test::NoNesting::Reflect ().mapping);
    const Graph expected = {"NoNesting",
                            {},
                            {},
                            {{Common::Constants::MAPPING_ROOT_NODE, {}}, {"x", {}}, {"y", {}}, {"z", {}}},
                            {{Common::Constants::MAPPING_ROOT_NODE, "x", {}},
                             {Common::Constants::MAPPING_ROOT_NODE, "y", {}},
                             {Common::Constants::MAPPING_ROOT_NODE, "z", {}}}};

    CHECK (result == expected);
}

TEST_CASE (OneNestingLevel)
{
    using namespace Emergence::VisualGraph;
    using namespace Emergence::StandardLayout;

    const Graph result = Visualization::GraphFromMapping (Test::OneNestingLevel::Reflect ().mapping);
    const Graph expected = {"OneNestingLevel",
                            {},
                            {},
                            {{Common::Constants::MAPPING_ROOT_NODE, {}},
                             {"first", {}},
                             {"first.x", "x"},
                             {"first.y", "y"},
                             {"first.z", "z"},
                             {"second", {}},
                             {"second.x", "x"},
                             {"second.y", "y"},
                             {"second.z", "z"}},
                            {{Common::Constants::MAPPING_ROOT_NODE, "first", {}},
                             {"first", "first.x", {}},
                             {"first", "first.y", {}},
                             {"first", "first.z", {}},
                             {Common::Constants::MAPPING_ROOT_NODE, "second", {}},
                             {"second", "second.x", {}},
                             {"second", "second.y", {}},
                             {"second", "second.z", {}}}};

    CHECK (result == expected);
}

TEST_CASE (TwoNestingLevels)
{
    using namespace Emergence::VisualGraph;
    using namespace Emergence::StandardLayout;

    const Graph result = Visualization::GraphFromMapping (Test::TwoNestingLevels::Reflect ().mapping);
    const Graph expected = {"TwoNestingLevels",
                            {},
                            {},
                            {{Common::Constants::MAPPING_ROOT_NODE, {}},
                             {"first", {}},
                             {"first.first", "first"},
                             {"first.first.x", "x"},
                             {"first.first.y", "y"},
                             {"first.first.z", "z"},
                             {"first.second", "second"},
                             {"first.second.x", "x"},
                             {"first.second.y", "y"},
                             {"first.second.z", "z"},
                             {"second", {}},
                             {"second.x", "x"},
                             {"second.y", "y"},
                             {"second.z", "z"}},
                            {{Common::Constants::MAPPING_ROOT_NODE, "first", {}},
                             {"first", "first.first", {}},
                             {"first.first", "first.first.x", {}},
                             {"first.first", "first.first.y", {}},
                             {"first.first", "first.first.z", {}},
                             {"first", "first.second", {}},
                             {"first.second", "first.second.x", {}},
                             {"first.second", "first.second.y", {}},
                             {"first.second", "first.second.z", {}},
                             {Common::Constants::MAPPING_ROOT_NODE, "second", {}},
                             {"second", "second.x", {}},
                             {"second", "second.y", {}},
                             {"second", "second.z", {}}}};

    CHECK (result == expected);
}

END_SUITE
